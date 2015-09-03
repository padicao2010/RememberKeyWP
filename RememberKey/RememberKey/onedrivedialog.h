#ifndef ONEDRIVEDIALOG_H
#define ONEDRIVEDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QList>
#include <QJsonObject>
#include "../QtOneDriveLib/qtonedrive.h"
#include "../QtAesLib/qtaes.h"

namespace Ui {
class OnedriveDialog;
}

struct DirInfo {
    DirInfo() :id(""), name(""), type(""), isDir(true) {}
    DirInfo(const QJsonObject &json);

    QString id;
    QString name;
    QString type;
    bool isDir;
};

class ODDirModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ODDirModel(QObject *parent = 0):QAbstractTableModel(parent) {}

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    const DirInfo &getDirInfo(int row);
    const DirInfo &getParentDirInfo();
    QString  getDirPath();

    void refresh();
    void goDown(const DirInfo &dir);
    bool goUp();
    void succeededOperation(const QList<DirInfo> &subdir);
    void failedOperation();

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<DirInfo> updownList;
    QList<DirInfo> childList;
};

class OnedriveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnedriveDialog(QSettings *set, QtAes *aes, QWidget *parent = 0);
    ~OnedriveDialog();

    bool getReady() { return ready; }
    void reset();
    void doAuth();
    void doUpload(QFile *file);
    void doDownload(QFile *file);
    QFile *getTemperoryFile();

signals:
    void successConfig();
    void statusDoUpload(QFile *file, bool success, const QString &msg);
    void statusDoDownload(QFile *file, bool success, const QString &msg);

public slots:
    void errorSignIn(const QString &err);
    void successSignIn();
    void tokenChange(const QString &access, const QString &refresh, const QDateTime &expired);
    void successTraverseFolder(const QJsonObject &json, const QString &rootFolderID);
    void errorTraverseFolder(const QString &error);
    void errorCreateFolder(const QString &err);
    void successCreateFolder(const QString &fold);
    void errorUploadFile(const QString &err);
    void successUploadFile(const QString &localFilePath, const QString &fileId);
    void errorDownloadFile(const QString &err);
    void successDownloadFile(const QString &fileId);
    void progressChange(const QString &str, int percent);

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_goupButton_clicked();
    void on_addFolderButton_clicked();
    void on_addFileButton_clicked();

    void on_goButton_clicked();

private:
    void loadSettings();
    void saveSettings();
    void loadFolder(const QString &folderid);
    void selectFile(const QString &foldid, const QString &filename, const QString &fileid);
    void activateProgress(bool up);

    void startSelectWaiting();
    void finishSelectWaiting();

private:
    Ui::OnedriveDialog *ui;
    QtOneDrive *onedrive;
    QtAes *cryptoAes;
    QSettings *settings;
    ODDirModel *dirModel;

    bool ready;

    QString folderID;
    QString fileID;
    QString fileName;

    QFile *tmp_file;
};

#endif // ONEDRIVEDIALOG_H
