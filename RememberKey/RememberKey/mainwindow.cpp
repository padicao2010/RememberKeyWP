#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QDesktopServices>

static const QString RK_LAST_SECTION = "rk.last.section";
static const QString RK_DATABASE_FILEPATH = "rk.main.database.filepath";
static const QString RK_CLIPBOARD_EXPIRE = "rk.main.clipboard.expire";
static const QString RK_APP_EXPIRE = "rk.main.app.expire";
static const QString RK_ONEDRIVE_ENABLED = "rk.main.onedrive.enable";

static const QString RK_APPLICATION_NAME = "RememberKey";
static const QString RK_ORGANIZATION_NAME = "www.yvanhom.com";
static const int RK_CLIPBOARD_TIMEOUT_DEFAULT = 10000;
static const int RK_APP_TIMEOUT_DEFAULT = 25000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralWidget->hide();

    // These exist in all application time
    appSettings = new QSettings(RK_APPLICATION_NAME, RK_ORGANIZATION_NAME);
    database = new KeyDatabase;
    createDialog = new CreateDialog(this);
    createDialog->setModal(true);
    editDialog = new EditDialog(this);
    editDialog->setModal(true);
    clipboardTimer = new QTimer(this);
    appTimer = new QTimer(this);

    this->installEventFilter(this);
    editDialog->installEventFilter(this);
    isOnedriveActive = false;

    // These will set at section time
    settings = nullptr;
    onedriveDialog = nullptr;
    cryptoAes = nullptr;

    // 计时器
    connect(clipboardTimer, &QTimer::timeout, this, &MainWindow::clipboard_timeout);
    connect(appTimer, &QTimer::timeout, this, &MainWindow::app_timeout);

    // Deal with SettingDialog signals: Create Database file
    connect(createDialog, &CreateDialog::inputCreateReady, this, &MainWindow::createDatabase);

    // Deal with EditDialog signals
    // Add record
    connect(editDialog, &EditDialog::addInputReady, this, &MainWindow::addKeyInfo);
    // Edit record
    connect(editDialog, &EditDialog::updateInputReady, this, &MainWindow::updateKeyInfo);

    if(loadSettings()) {
        if(loadSectionSettings()) {
            activeMainWindow();
        } else {
            closeSection();
        }
    }
}

MainWindow::~MainWindow()
{
    closeSection();
    editDialog->deleteLater();
    createDialog->deleteLater();
    appSettings->deleteLater();
    delete database;
    delete ui;
}

void MainWindow::on_action_New_triggered()
{
    createDialog->show();
}

void MainWindow::createDatabase(const QString &path, const QString &pass, bool onedriveActive)
{
    closeSection();

    // create database
    settings = new QSettings(path, QSettings::IniFormat);
    cryptoAes = new QtAes;
    cryptoAes->initialize(pass);
    QString dbpath = path + ".db";
    if(!database->create(dbpath, pass, cryptoAes)) {
        warnError(createDialog, database->getLastErrorMessage());
        return;
    }

    // active mainwindow when success
    clipTimeout = RK_CLIPBOARD_TIMEOUT_DEFAULT;
    appTimeout = RK_APP_TIMEOUT_DEFAULT;
    clipboardTimer->setInterval(clipTimeout);
    appTimer->setInterval(appTimeout);

    saveSectionSettings();
    saveSettings();

    createDialog->close();
    activeMainWindow();

    // initialize onedrive if need
    if(onedriveActive) {
        activeOnedrive();
    }
}

void MainWindow::on_action_Open_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open a RememberKey Config file"),
        "", tr("SQLite File(*.rkini)"));
    if(filename.isEmpty()) {
        return;
    }

    closeSection();

    settings = new QSettings(filename, QSettings::IniFormat);

    if(!loadSectionSettings()) {
        closeSection();
        return;
    }

    saveSettings();;

    activeMainWindow();
}

bool MainWindow::activePassword()
{
    QString tipMsg = tr("Input Password: ");
    for(int i = 0; i < 3; i++) {
        QString password = QInputDialog::getText(this, tr(""),
            tipMsg, QLineEdit::Password);

        if(password.isEmpty()) {
            database->close();
            return false;
        } else {
            cryptoAes = new QtAes;
            cryptoAes->initialize(password);
            if(database->activePassword(password, cryptoAes)) {
                return true;
            } else {
                delete cryptoAes;
                cryptoAes = nullptr;
                tipMsg = QString("%1\nInput Password: ").arg(database->getLastErrorMessage());
            }
        }
    }
    warnError(this, tr("Sorry! You have failed 3 times"));
    return false;
}

void MainWindow::activeMainWindow()
{
    ui->itemsTable->setModel(database->getQueryModel());
    ui->itemsTable->setColumnHidden(0, true);
    ui->itemsTable->setColumnHidden(3, true);
    //ui->itemsTable->resize
    ui->centralWidget->show();
    ui->menuEdit->setEnabled(true);
    ui->menuOneDrive->setEnabled(true);
    updateOnedriveMenu();
    updateEditMenu(false);

    appTimer->start();
    appActive = false;
}

void MainWindow::deactiveMainWindow()
{
    ui->centralWidget->hide();
    ui->menuEdit->setEnabled(false);
    ui->menuOneDrive->setEnabled(false);
    appTimer->stop();
}

void MainWindow::warnError(QWidget *parent, const QString &errMsg)
{
    QMessageBox::warning(parent, tr("Error happen"), errMsg);
}

void MainWindow::on_searchEdit_returnPressed()
{
    if(!database->search(ui->searchEdit->text())) {
        warnError(this, database->getLastErrorMessage());
    }
}

void MainWindow::on_actionAdd_triggered()
{
    editDialog->setInfo(true, NULL);
    editDialog->show();
}

void MainWindow::addKeyInfo(const KeyInfo &key)
{
    if(!database->addKeyInfo(key)) {
        warnError(editDialog, database->getLastErrorMessage());
        return;
    }
    database->updateQueryModel(key.getName());

    editDialog->close();
    updateEditMenu(false);
}

void MainWindow::on_actionEdit_triggered()
{
    startEditDialog(-1);
}

void MainWindow::on_itemsTable_doubleClicked(const QModelIndex &index)
{
    startEditDialog(index.row());
}

void MainWindow::updateKeyInfo(const KeyInfo &old, const KeyInfo &key)
{
    if(!database->updateKeyInfo(old, key)) {
        warnError(editDialog, database->getLastErrorMessage());
        return;
    }
    database->updateQueryModel(key.getName());

    editDialog->close();
    updateEditMenu(false);
}

void MainWindow::onedriveDialogFinished(int result)
{
    qDebug() << "Onedrive Dialog closed : " << result;
    if(!onedriveDialog->getReady()) {
        delete onedriveDialog;
        onedriveDialog = nullptr;
    }
    appTimer->start();
    updateOnedriveMenu();
}

void MainWindow::onedriveSuccessConfig()
{
    isOnedriveActive = true;
    saveSectionSettings();
    onedriveDialog->close();
}

void MainWindow::onedriveUploadStatus(QFile *file, bool status, const QString &msg)
{
    onedriveDialog->close();
    qDebug() << "Upload finished";
    file->deleteLater();
    //onedriveDialog->close();
    if(status) {
        QMessageBox::information(this, tr("Operation success"), tr("Upload DONE"));
    } else {
        QMessageBox::warning(this, tr("Operation failed"), tr("Upload FAILED : %1").arg(msg));
    }
}

void MainWindow::onedriveDownloadStatus(QFile *file, bool status, const QString &msg)
{
    qDebug() << "Download finished";

    file->seek(0);
    if(status) {
        if(!database->importFromFile(file)) {
            onedriveDialog->close();
            QMessageBox::warning(this, tr("Error"), tr("Can't import file : %1").arg(database->getLastErrorMessage()));
            database->updateQueryModel("");
        } else {
            onedriveDialog->close();
            database->updateQueryModel("");
            //onedriveDialog->close();
            QMessageBox::information(this, tr("Operation success"), tr("Download and update database DONE"));
        }
    } else {
        onedriveDialog->close();
        QMessageBox::warning(this, tr("Operation failed"), tr("Can't upload file : %1").arg(msg));
    }

    file->deleteLater();
}

void MainWindow::on_actionDelete_triggered()
{
    KeyInfo key;
    getSelectedKeyInfo(-1, &key);

    if(!database->deleteKeyInfo(key.getId())) {
        warnError(this, database->getLastErrorMessage());
        return;
    }
    database->updateQueryModel("");
    updateEditMenu(false);
}

void MainWindow::startEditDialog(int row)
{
    KeyInfo key;
    getSelectedKeyInfo(row, &key);
    editDialog->setInfo(false, &key);
    editDialog->show();
}

void MainWindow::on_actionCopy_Password_triggered()
{
    QClipboard *board = QApplication::clipboard();
    KeyInfo key;
    getSelectedKeyInfo(-1, &key);
    board->setText(key.getPassword());

    clipboardTimer->start();
}

void MainWindow::updateEditMenu(bool choose)
{
    ui->actionEdit->setEnabled(choose);
    ui->actionDelete->setEnabled(choose);
    ui->actionCopy_Password->setEnabled(choose);
    ui->actionCopy_Username->setEnabled(choose);
}

void MainWindow::updateOnedriveMenu()
{
    ui->actionActivate->setChecked(isOnedriveActive);
    ui->actionUpload->setEnabled(isOnedriveActive);
    ui->actionDownload->setEnabled(isOnedriveActive);
}

bool MainWindow::loadSettings()
{
    QString last = appSettings->value(RK_LAST_SECTION, "").toString();
    if(last.isEmpty()) {
        qDebug() << "No last section";
        return false;
    }

    if(!QFile(last).exists()) {
        qDebug() << "Last section file not exist : " << last;
        return false;
    }

    settings = new QSettings(last, QSettings::IniFormat);
    return true;
}

void MainWindow::saveSettings()
{
    appSettings->setValue(RK_LAST_SECTION, settings->fileName());
}

void MainWindow::saveSectionSettings()
{
    settings->setValue(RK_DATABASE_FILEPATH, database->getFilePath());
    settings->setValue(RK_CLIPBOARD_EXPIRE, clipTimeout);
    settings->setValue(RK_APP_EXPIRE, appTimeout);
    settings->setValue(RK_ONEDRIVE_ENABLED, isOnedriveActive);
}

void MainWindow::closeSection()
{
    database->close();
    if(onedriveDialog != nullptr) {
        delete onedriveDialog;
        onedriveDialog = nullptr;
    }
    if(cryptoAes != nullptr) {
        delete cryptoAes;
        cryptoAes = nullptr;
    }
    if(settings != nullptr) {
        delete settings;
        settings = nullptr;
    }
}

void MainWindow::createOnedrive()
{
    onedriveDialog = new OnedriveDialog(settings, cryptoAes);
    onedriveDialog->setModal(true);
    connect(onedriveDialog, &OnedriveDialog::successConfig, this, &MainWindow::onedriveSuccessConfig);
    connect(onedriveDialog, &OnedriveDialog::statusDoDownload, this, &MainWindow::onedriveDownloadStatus);
    connect(onedriveDialog, &OnedriveDialog::statusDoUpload, this, &MainWindow::onedriveUploadStatus);
    connect(onedriveDialog, &OnedriveDialog::finished,  this, &MainWindow::onedriveDialogFinished);
}

void MainWindow::activeOnedrive()
{
    appTimer->stop();
    createOnedrive();
    onedriveDialog->doAuth();
    onedriveDialog->show();
}

void MainWindow::deactiveOnedrive()
{
    isOnedriveActive = false;
    saveSectionSettings();
    onedriveDialog->reset();
    delete onedriveDialog;
    onedriveDialog = nullptr;
}

bool MainWindow::loadSectionSettings()
{
    QString filepath = settings->value(RK_DATABASE_FILEPATH, "").toString();
    if(filepath.isEmpty()) {
        qDebug() << "No database file";
        return false;
    }
    if(!QFile(filepath).exists()) {
        qDebug() << "Database file not exist: " << filepath;
        return false;
    }
    clipTimeout = settings->value(RK_CLIPBOARD_EXPIRE, RK_CLIPBOARD_TIMEOUT_DEFAULT).toInt();
    clipboardTimer->setInterval(clipTimeout);
    appTimeout = settings->value(RK_CLIPBOARD_EXPIRE, RK_APP_TIMEOUT_DEFAULT).toInt();
    appTimer->setInterval(appTimeout);
    if(!database->open(filepath)) {
        qDebug() << "Can't open database";
        return false;
    }

    if(!activePassword()) {
        qDebug() << "Can't active password";
        return false;
    }

    isOnedriveActive = settings->value(RK_ONEDRIVE_ENABLED, false).toBool();
    if(isOnedriveActive) {
        createOnedrive();
        if(!onedriveDialog->getReady()) {
            isOnedriveActive = false;
            delete onedriveDialog;
            onedriveDialog = nullptr;
        }
    }

    return true;
}

void MainWindow::forgetPassword()
{
    if(createDialog->isVisible()) {
        createDialog->close();
    }
    if(editDialog->isVisible()) {
        editDialog->close();
    }
    deactiveMainWindow();

    database->forgetPassword();
}

void MainWindow::getSelectedKeyInfo(int row, KeyInfo *key)
{
    if(row == -1) {
        QItemSelectionModel *select = ui->itemsTable->selectionModel();
        row = select->currentIndex().row();
    }

    database->getKeyInQueryModel(row, key);
}

void MainWindow::on_itemsTable_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->itemsTable->indexAt(pos);
    if(index.row() >= 0) {
        updateEditMenu(true);
    }

    ui->menuEdit->move(cursor().pos());
    ui->menuEdit->show();
}

void MainWindow::on_itemsTable_clicked(const QModelIndex &index)
{
    qDebug() << "Click " << index.row();
    updateEditMenu(true);
}

void MainWindow::on_actionCopy_Username_triggered()
{
    QClipboard *board = QApplication::clipboard();
    KeyInfo key;
    getSelectedKeyInfo(-1, &key);
    board->setText(key.getUsername());

    clipboardTimer->start();
}

void MainWindow::clipboard_timeout()
{
    qDebug() << "clipboard timeout";
    QClipboard *board = QApplication::clipboard();
    board->clear();

    clipboardTimer->stop();
}

void MainWindow::app_timeout()
{
    qDebug() << "Application state " << appActive;
    if(!appActive) {
        qDebug() << "Try to forget password";
        forgetPassword();
        if(activePassword()) {
            activeMainWindow();
        }
    }
    appActive = false;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() != QEvent::Timer &&
            event->type() != QEvent::Paint &&
            event->type() != QEvent::UpdateRequest) {
        if(!appActive) {
            qDebug() << "Watched " << watched->objectName();
            appActive = true;
        }
    }
    return false;
}

void MainWindow::on_actionActivate_triggered(bool checked)
{
    if(checked == isOnedriveActive) {
        qDebug() << QString("checked is %1, equals to isOnedriveActive").arg(checked);
        ui->actionActivate->setChecked(isOnedriveActive);
        return;
    }

    if(isOnedriveActive) {
        deactiveOnedrive();
        updateOnedriveMenu();
    } else {
        activeOnedrive();
    }
}

void MainWindow::on_actionUpload_triggered()
{
    QTemporaryFile *tempFile = new QTemporaryFile;
    if(!tempFile->open()) {
        QMessageBox::warning(this, tr("Error"), tr("Can't open temperary file for upload purpose"));
        tempFile->deleteLater();
        return;
    }
    qDebug() << tempFile->fileName();

    if(!database->exportToFile(tempFile)) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to export database : %1").arg(database->getLastErrorMessage()));
        return;
    }

    tempFile->seek(0);

    appTimer->stop();
    onedriveDialog->doUpload(tempFile);
    onedriveDialog->show();
}

void MainWindow::on_actionDownload_triggered()
{
    QTemporaryFile *tempFile = new QTemporaryFile;
    if(!tempFile->open()) {
        QMessageBox::warning(this, tr("Error"), tr("Can't open temperary file for download purpose"));
        tempFile->deleteLater();
        return;
    }
    qDebug() << tempFile->fileName();

    appTimer->stop();
    onedriveDialog->doDownload(tempFile);
    onedriveDialog->show();
}
