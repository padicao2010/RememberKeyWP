#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QTimer>
#include <QDateTime>
#include <QUrl>
#include <QSettings>
#include "keydatabase.h"
#include "createdialog.h"
#include "editdialog.h"
#include "onedrivedialog.h"
#include "../QtOneDriveLib/qtonedrive.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void createDatabase(const QString& path, const QString &pass, bool onedrive);
    void addKeyInfo(const KeyInfo &key);
    void updateKeyInfo(const KeyInfo &old, const KeyInfo &key);

    void onedriveDialogFinished(int result);
    void onedriveSuccessConfig();
    void onedriveUploadStatus(QFile *file, bool status, const QString &msg);
    void onedriveDownloadStatus(QFile *file, bool status, const QString &msg);

private slots:
    // used for UI
    void on_searchEdit_returnPressed();
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_itemsTable_doubleClicked(const QModelIndex &index);
    void on_itemsTable_customContextMenuRequested(const QPoint &pos);
    void on_actionAdd_triggered();
    void on_itemsTable_clicked(const QModelIndex &index);
    void on_actionEdit_triggered();
    void on_actionDelete_triggered();
    void on_actionCopy_Password_triggered();
    void on_actionCopy_Username_triggered();
    void on_actionActivate_triggered(bool checked);
    void on_actionUpload_triggered();
    void on_actionDownload_triggered();

    // used for Timer
    bool eventFilter(QObject *, QEvent *);
    void clipboard_timeout();
    void app_timeout();

private:
    void warnError(QWidget *parent, const QString &errMsg);
    void getSelectedKeyInfo(int row, KeyInfo *key);
    void startEditDialog(int row);

    bool activePassword();
    void forgetPassword();

    void activeMainWindow();
    void deactiveMainWindow();
    void updateEditMenu(bool choose);
    void updateOnedriveMenu();

    bool loadSettings();
    void saveSettings();
    bool loadSectionSettings();
    void saveSectionSettings();
    void closeSection();

    void createOnedrive();
    void activeOnedrive();
    void deactiveOnedrive();

    QSettings getApplicationSettings();

    Ui::MainWindow *ui;
    CreateDialog *createDialog;
    EditDialog *editDialog;
    OnedriveDialog *onedriveDialog;

    QTimer *clipboardTimer;
    int clipTimeout;
    QTimer *appTimer;
    int appTimeout;
    bool appActive;

    KeyDatabase *database;

    bool isOnedriveActive;

    QSettings *appSettings;
    QSettings *settings;
    QtAes *cryptoAes;
};

#endif // MAINWINDOW_H
