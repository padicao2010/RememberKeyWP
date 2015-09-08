#include "onedrivedialog.h"
#include "ui_onedrivedialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QDesktopServices>

static const QString RK_ONEDRIVE_CLIENT_ID = "rk.onedrive.client.id";
static const QString RK_ONEDRIVE_SECRET_KEY = "rk.onedrive.secret.key";
static const QString RK_ONEDRIVE_REDIRECT_URL = "rk.onedrive.redirect.url";
static const QString RK_ONEDRIVE_ACCESS_TOKEN = "rk.onedrive.access.token";
static const QString RK_ONEDRIVE_REFRESH_TOKEN = "rk.onedrive.refresh.token";
static const QString RK_ONEDRIVE_EXPIRED_TIME = "rk.onedrive.expired.time";
static const QString RK_ONEDRIVE_FOLDER_ID = "rk.onedrive.folder.id";
static const QString RK_ONEDRIVE_FILE_NAME = "rk.onedrive.file.name";
static const QString RK_ONEDRIVE_FILE_ID = "rk.onedrive.file.id";

OnedriveDialog::OnedriveDialog(QSettings *set, QtAes *aes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnedriveDialog)
{
    ui->setupUi(this);
    onedrive = nullptr;

    cryptoAes = aes;
    settings = set;
    dirModel = new ODDirModel(this);
    ui->tableView->setModel(dirModel);
    ready = false;

    loadSettings();

    connect(onedrive, &QtOneDrive::successSignIn, this, &OnedriveDialog::successSignIn);
    connect(onedrive, &QtOneDrive::errorSignIn, this, &OnedriveDialog::errorSignIn);
    connect(onedrive, &QtOneDrive::tokenChange, this, &OnedriveDialog::tokenChange);
    connect(onedrive, &QtOneDrive::successTraverseFolder, this, &OnedriveDialog::successTraverseFolder);
    connect(onedrive, &QtOneDrive::errorTraverseFolder, this, &OnedriveDialog::errorTraverseFolder);
    connect(onedrive, &QtOneDrive::successCreateFolder, this, &OnedriveDialog::successCreateFolder);
    connect(onedrive, &QtOneDrive::errorCreateFolder, this, &OnedriveDialog::errorCreateFolder);
    connect(onedrive, &QtOneDrive::successUploadFile, this, &OnedriveDialog::successUploadFile);
    connect(onedrive, &QtOneDrive::errorUploadFile, this, &OnedriveDialog::errorUploadFile);
    connect(onedrive, &QtOneDrive::successDownloadFile, this, &OnedriveDialog::successDownloadFile);
    connect(onedrive, &QtOneDrive::errorDownloadFile, this, &OnedriveDialog::errorDownloadFile);
    connect(onedrive, &QtOneDrive::progressUploadFile, this, &OnedriveDialog::progressChange);
    connect(onedrive, &QtOneDrive::progressDownloadFile, this, &OnedriveDialog::progressChange);
}

OnedriveDialog::~OnedriveDialog()
{
    delete ui;
    delete onedrive;
    cryptoAes = nullptr;
    settings = nullptr;
}

void OnedriveDialog::reset()
{
    if(ready) {
        ready = false;
        if(onedrive->isSingIn()) {
            onedrive->signOut();
        }
        saveSettings();
    }
}

void OnedriveDialog::doAuth()
{
    ui->tabSighIn->setEnabled(true);
    ui->tabSelect->setEnabled(false);
    ui->tabProgress->setEnabled(false);
    ui->tabWidget->setCurrentIndex(0);

    if(!ready) {
        QDesktopServices::openUrl(onedrive->getAuthorizationUrl());
    }
}

void OnedriveDialog::errorSignIn(const QString &err)
{
    QMessageBox::warning(this, tr("Operation failed"), tr("Can't sign in for %1").arg(err));
}

void OnedriveDialog::successSignIn()
{
    ui->tabSighIn->setEnabled(false);
    ui->tabSelect->setEnabled(true);
    ui->tabWidget->setCurrentIndex(1);

    loadFolder("");
}

void OnedriveDialog::tokenChange(const QString &access, const QString &refresh, const QDateTime &expired)
{
    if(ready) {
        qDebug() << "TOKEN: " << access << " | " << refresh << " | " << expired.toString(Qt::ISODate);
        saveSettings();
    }
}

void OnedriveDialog::successTraverseFolder(const QJsonObject &json, const QString &rootFolderID)
{
    qDebug() << "Root ID" << rootFolderID;
    if(rootFolderID == "") {
        DirInfo dir(json);
        QList<DirInfo> dirlist;
        dirlist.append(dir);
        dirModel->succeededOperation(dirlist);
    } else {
        const QJsonArray &array = json.value("data").toArray();
        QList<DirInfo> dirlist;

        for(auto iter = array.constBegin(); iter != array.constEnd(); iter++) {
            QJsonObject jo = (*iter).toObject();
            DirInfo dir(jo);
            if(!dir.isDir && dir.type != "file") {
                // Skip those items
                continue;
            }
            dirlist.append(dir);
            qDebug() << "ADD " << dir.id;
        }
        dirModel->succeededOperation(dirlist);
    }
    finishSelectWaiting();
}

void OnedriveDialog::errorTraverseFolder(const QString &error)
{
    QMessageBox::warning(this, tr("Operation Error"), tr("Can't traverse folder : %1").arg(error));
    dirModel->failedOperation();
    finishSelectWaiting();
}

void OnedriveDialog::errorCreateFolder(const QString &err)
{
    QMessageBox::warning(this, tr("Operation Error"), tr("Can't create folder : %1").arg(err));
    finishSelectWaiting();
}

void OnedriveDialog::successCreateFolder(const QString &fold)
{
    qDebug() << "New folder ID :" << fold;
    const DirInfo &dir = dirModel->getParentDirInfo();
    dirModel->refresh();
    onedrive->traverseFolder(dir.id);
}

void OnedriveDialog::errorUploadFile(const QString &err)
{
    if(!ready) {
        QMessageBox::warning(this, tr("Create File Error"), tr("Can't create file : %1").arg(err));
        finishSelectWaiting();
    } else {
        //QMessageBox::warning(this, tr("Upload File Error"), tr("Can't upload file : %1").arg(err));
        //this->hide();
        emit statusDoDownload(tmp_file, false, err);
    }
}

void OnedriveDialog::successUploadFile(const QString &remoteFileName, const QString &fileId)
{
    qDebug() << "New file ID :" << fileId;
    //const DirInfo &dir = dirModel->getParentDirInfo();
    //dirModel->refresh();
    //onedrive->traverseFolder(dir.id);
    if(!ready) {
        // create file in select file period
        selectFile(dirModel->getParentDirInfo().id, fileId, remoteFileName);
    } else {
        //QMessageBox::information(this, tr("Operation Succeed"), tr("Upload file succeed"));
        //this->hide();
        emit statusDoUpload(tmp_file, true, "");
    }
}

void OnedriveDialog::errorDownloadFile(const QString &err)
{
    //QMessageBox::warning(this, tr("Download File Error"), tr("Can't download file : %1").arg(err));
    //this->hide();
    emit statusDoDownload(tmp_file, false, err);
}

void OnedriveDialog::successDownloadFile(const QString &fileId)
{
    qDebug() << "Success download " << fileId;
    emit statusDoDownload(tmp_file, true, "");
}

void OnedriveDialog::progressChange(const QString &str, int percent)
{
    qDebug() << "progress change " << str;
    ui->progressBar->setValue(percent);
}

void OnedriveDialog::loadSettings()
{
    QString clientID = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_CLIENT_ID, "").toString());
    QString secret = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_SECRET_KEY, "").toString());
    QString redirectUrl = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_REDIRECT_URL, "").toString());
    QString accessToken = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_ACCESS_TOKEN, "").toString());
    QString refreshToken = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_REFRESH_TOKEN, "").toString());
    QString expired = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_EXPIRED_TIME, "").toString());
    folderID = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_FOLDER_ID, "").toString());
    fileID = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_FILE_ID, "").toString());
    fileName = cryptoAes->decrypt(settings->value(RK_ONEDRIVE_FILE_NAME, "").toString());

    if(clientID.isEmpty() || secret.isEmpty() || redirectUrl.isEmpty() || accessToken.isEmpty() || refreshToken.isEmpty() ||
            expired.isEmpty() || folderID.isEmpty() || fileID.isEmpty() || fileName.isEmpty()) {
        onedrive = new QtOneDrive(
                    tr("0000000040167206"),
                    tr("a7VxYh/9l2o83XKh+1Sb1HA+FfMd/6re"),
                    tr("https://login.live.com/oauth20_desktop.srf"));
        ready = false;
    } else {
        onedrive = new QtOneDrive(clientID, secret, redirectUrl);
        onedrive->setToken(accessToken,refreshToken, QDateTime::fromString(expired, Qt::ISODate));
        ready = true;
    }
}

void OnedriveDialog::saveSettings()
{
    if(ready == false || onedrive == nullptr) {
        settings->remove(RK_ONEDRIVE_CLIENT_ID);
        settings->remove(RK_ONEDRIVE_SECRET_KEY);
        settings->remove(RK_ONEDRIVE_REDIRECT_URL);
        settings->remove(RK_ONEDRIVE_ACCESS_TOKEN);
        settings->remove(RK_ONEDRIVE_REFRESH_TOKEN);
        settings->remove(RK_ONEDRIVE_EXPIRED_TIME);
        settings->remove(RK_ONEDRIVE_FOLDER_ID);
        settings->remove(RK_ONEDRIVE_FILE_ID);
        settings->remove(RK_ONEDRIVE_FILE_NAME);
    } else {
        settings->setValue(RK_ONEDRIVE_CLIENT_ID, cryptoAes->encrypt(onedrive->getClientID()));
        settings->setValue(RK_ONEDRIVE_SECRET_KEY, cryptoAes->encrypt(onedrive->getSecretKey()));
        settings->setValue(RK_ONEDRIVE_REDIRECT_URL, cryptoAes->encrypt(onedrive->getRedirectUrl()));
        settings->setValue(RK_ONEDRIVE_ACCESS_TOKEN, cryptoAes->encrypt(onedrive->getAccessToken()));
        settings->setValue(RK_ONEDRIVE_REFRESH_TOKEN, cryptoAes->encrypt(onedrive->getRefreshToken()));
        settings->setValue(RK_ONEDRIVE_EXPIRED_TIME, cryptoAes->encrypt(onedrive->getExpiredTime().toString(Qt::ISODate)));
        settings->setValue(RK_ONEDRIVE_FOLDER_ID, cryptoAes->encrypt(folderID));
        settings->setValue(RK_ONEDRIVE_FILE_ID, cryptoAes->encrypt(fileID));
        settings->setValue(RK_ONEDRIVE_FILE_NAME, cryptoAes->encrypt(fileName));
    }
}

void OnedriveDialog::loadFolder(const QString &folderid)
{
    startSelectWaiting();
    dirModel->goDown(DirInfo());
    onedrive->traverseFolder(folderid);
}

void OnedriveDialog::selectFile(const QString &foldid, const QString &fileid, const QString &filename)
{
    this->folderID = foldid;
    this->fileName = filename;
    this->fileID = fileid;

    qDebug() << "Selected Folder ID : " << foldid;
    qDebug() << "Selected File ID : " << filename;
    qDebug() << "Selected File Name : " << fileid;

    this->ready = true;
    saveSettings();
    emit successConfig();
}

void OnedriveDialog::startSelectWaiting()
{
    ui->tableView->setEnabled(false);
    ui->goupButton->setEnabled(false);
    ui->addFileButton->setEnabled(false);
    ui->addFolderButton->setEnabled(false);
}

void OnedriveDialog::finishSelectWaiting()
{
    ui->tableView->setEnabled(true);
    const DirInfo &dir = dirModel->getParentDirInfo();
    if(!dir.id.isEmpty()) {
        ui->goupButton->setEnabled(true);
        ui->addFileButton->setEnabled(true);
        ui->addFolderButton->setEnabled(true);
    }
}

int ODDirModel::rowCount(const QModelIndex &parent) const
{
    //if(parent.isValid()) {
        return childList.size();
    //} else {
    //    return 0;
    //}
}

int ODDirModel::columnCount(const QModelIndex &parent) const
{
    //if(parent.isValid()) {
        return 2;
   // } else {
    //    return 0;
    //}
}

const DirInfo &ODDirModel::getDirInfo(int row)
{
    return childList.at(row);
}

const DirInfo &ODDirModel::getParentDirInfo()
{
    return updownList.at(updownList.size() - 1);
}

void ODDirModel::refresh()
{
    childList.clear();
    this->beginResetModel();
}

void ODDirModel::goDown(const DirInfo &dir)
{
    updownList.append(dir);
    childList.clear();
    this->beginResetModel();
}

bool ODDirModel::goUp()
{
    if(updownList.size() > 1) {
        this->beginResetModel();
        updownList.removeLast();
        childList.clear();
        return true;
    }
    return false;
}

void ODDirModel::failedOperation()
{
    this->endResetModel();
}

void ODDirModel::succeededOperation(const QList<DirInfo> &subdir)
{
    for(auto iter = subdir.cbegin(); iter != subdir.cend(); iter++) {
        childList.append(*iter);
    }

    this->endResetModel();
}

QString ODDirModel::getDirPath()
{
    QString result;
    for(auto iter = updownList.cbegin(); iter != updownList.cend(); iter++) {
        result = QString("%1/%2").arg(result).arg(iter->name);
    }
    return result;
}

QVariant ODDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        int row = index.row();
        int column = index.column();

        if(row < 0 || row >= childList.size()) {
            return QVariant();
        }
        const DirInfo &dir = childList.at(row);

        if(column < 0 || column >= 2) {
            return QVariant();
        }
        switch(column) {
        case 0:
            return dir.isDir ? " + " : " ";

        case 1:
            return dir.name;
        }

    }
    return QVariant();
}

QVariant ODDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if(orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return tr("Is Folder");
            case 1:
                return tr("Name");
            }
        }
    }
    return QVariant();
}

void OnedriveDialog::on_tableView_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    DirInfo dir = dirModel->getDirInfo(row);
    if(dir.isDir) {
        startSelectWaiting();
        dirModel->goDown(dir);
        qDebug() << "Begin traverse folder " << dir.name << " " << dir.id;
        onedrive->traverseFolder(dir.id);
    } else {
        if(dir.name.endsWith(".txt")) {
            selectFile(dirModel->getParentDirInfo().id, dir.id, dir.name);
        } else {
            QMessageBox::warning(this, tr("Select File Error"), tr("Should select a text file ends with .txt"));
        }
    }
}

void OnedriveDialog::on_goupButton_clicked()
{
    if(dirModel->goUp()) {
        startSelectWaiting();
        const DirInfo &dir = dirModel->getParentDirInfo();
        onedrive->traverseFolder(dir.id);
    }
}

DirInfo::DirInfo(const QJsonObject &json)
{
    id = json.value("id").toString();
    name = json.value("name").toString();
    type = json.value("type").toString();
    isDir = type == "folder";
}

void OnedriveDialog::on_addFolderButton_clicked()
{
    QString fold = QInputDialog::getText(this, tr("Add Folder"), tr("Input a folder name"));
    if(fold.isEmpty()) {
        return;
    }

    startSelectWaiting();
    onedrive->createFolder(fold, dirModel->getParentDirInfo().id);
}

void OnedriveDialog::on_addFileButton_clicked()
{
    QString filename = QInputDialog::getText(this, tr("Add File and select"), tr("Input a file name(the file name will be end with .txt)"));
    if(filename.isEmpty()) {
        return;
    }
    if(!filename.endsWith(".txt")) {
        filename = filename + ".txt";
    }

    qDebug() << "create file " + filename;

    const DirInfo &dir = dirModel->getParentDirInfo();
    startSelectWaiting();
    QByteArray data;
    onedrive->uploadFile(data, filename, dir.id);
}

void OnedriveDialog::doUpload(QFile *file)
{
    tmp_file = file;
    activateProgress(true);
    onedrive->uploadFile(file, fileName, folderID);
}

void OnedriveDialog::doDownload(QFile *file)
{
    tmp_file = file;
    activateProgress(false);
    onedrive->downloadFile(file, fileID);
}

void OnedriveDialog::activateProgress(bool up)
{
    ui->tabSighIn->setEnabled(false);
    ui->tabSelect->setEnabled(false);
    ui->tabProgress->setEnabled(true);
    ui->tabWidget->setCurrentIndex(2);

    ui->progressBar->setRange(0, 100);
    ui->downloadLabel->setVisible(!up);
    ui->uploadLabel->setVisible(up);
}

void OnedriveDialog::on_goButton_clicked()
{
    QString code = ui->codeInput->text();
    if(code.isEmpty()) {
        QMessageBox::warning(this, tr("Operation failed"), tr("Empty Code"));
        return;
    }
    onedrive->getTokenRequest(code);
}
