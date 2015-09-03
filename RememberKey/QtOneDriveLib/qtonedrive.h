#ifndef QTONEDRIVE_H
#define QTONEDRIVE_H

#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QFile>


class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class  QtOneDrive : public QObject
{
    Q_OBJECT

    enum OneDriveState
    {
        Empty = 0,

        SingIn,
        SingOut,
        GetUserInfo,
        RefreshToken,

        TraverseFolder,

        UploadFile,
        DownloadFile,

        DeleteItem,
        GetStorageInfo,
        CreateFolder
    };

public:
    explicit QtOneDrive(const QString& clientID,
                        const QString& secret,
                        const QString& redirectUri,
                        QObject* parent = 0 );

    ~QtOneDrive();

    const QString &getClientID() { return clientID_; }
    const QString &getSecretKey() { return secret_; }
    const QString &getRedirectUrl() { return redirectUri_; }
    const QString &getAccessToken() { return accessToken_; }
    const QString &getRefreshToken() { return refreshToken_; }
    const QDateTime &getExpiredTime() { return expiredTokenTime_; }


public:
    QUrl getAuthorizationUrl();
    void getCodeAndToken(const QUrl &url);
    void setToken(const QString& access,
                  const QString& refresh,
                  const QDateTime& expire);

    bool isSingIn() const { return isSignIn_; }

    void signOut();
    void getUserInfo();
    void refreshToken();
    void traverseFolder(const QString& rootFolderID = "");

    void getStorageInfo();

    void uploadFile(const QString& localFilePath, const QString& remoteFileName, const QString& folderID = "" );
    void uploadFile(QFile *file, const QString& remoteFileName, const QString& folderID = "" );
    void uploadFile(const QByteArray &data, const QString& remoteFileName, const QString& folderID = "" );
    void downloadFile(const QString& localFilePath, const QString& fileID);
    void downloadFile(QFile *file, const QString& fileID);

    void deleteItem(const QString& fileOrFolderID);
    void createFolder(const QString& folderName, const QString& parentFolderId = "");

    void getTokenRequest(const QString &authCode);

    bool isBusy() const { return state_ != Empty; }

signals:
    void errorSignIn( const QString &error );
    void errorSignOut( const QString &error );
    void errorUploadFile( const QString &error );
    void errorDownloadFile( const QString &error );
    void errorGetUserInfo( const QString &error );
    void errorRefreshToken( const QString &error );
    void errorTraverseFolder( const QString& error );
    void errorDeleteItem( const QString &error );
    void errorCreateFolder( const QString &error );
    void errorGetStorageInfo( const QString &error );
    void error( const QString &error );

    void progressUploadFile(const QString &localFilePath, int percent);
    void progressDownloadFile(const QString &fileID, int percent );

    void successSignIn();
    void successSingOut();

    void successUploadFile(const QString &remoteFileName, const QString &fileId);
    void successDownloadFile(const QString &fileID);
    void successDeleteItem(const QString &id);
    void successCreateFolder(const QString &folderId);

    void successRefreshToken();
    void successGetUserInfo(const QJsonObject &json);
    void successTraverseFolder(const QJsonObject &json, const QString &rootFolderID);
    void successGetStorageInfo(const QJsonObject &json);


    void tokenChange(const QString &access, const QString &refresh, const QDateTime &expire);

private:
    QUrl urlSignIn() const;
    QUrl urlSignOut() const;
    QUrl urlStorageInfo() const;
    QUrl urlGetToken() const;
    QUrl urlGetUserInfo() const;
    QUrl urlDeleteItem(const QString &id) const;
    QUrl urlTraverseFolder(const QString& parentFolderId) const;
    QUrl urlUploadFile( const QString& remoteFileName, const QString& folderId ) const;
    QUrl urlDownloadFile( const QString& fileId ) const;
    QUrl urlCreateFolder(const QString &id) const;

    QUrlQuery postGetToken(const QString &authCode) const;
    QUrlQuery postRefreshToken() const;
    QUrlQuery postCreateFolder(const QString &foldName) const;

    void onRefreshTokenFinished();
private:
    void emitError( const QString& desc );
    bool isNeedRefreshToken() const;
    void refreshTokenRequest();
    void downloadFile(const QUrl& url);

    QJsonObject checkReplyJson(QNetworkReply* reply);

private:
    QString clientID_;
    QString secret_;
    QString redirectUri_;
    QString accessToken_;
    QString refreshToken_;

    QDateTime tokenTime_;
    QDateTime expiredTokenTime_;

    bool isSignIn_ = false;

    OneDriveState state_ = Empty;
    QNetworkAccessManager* networkManager_ = nullptr;

    QString tmp_parentFolderId_;
    QString tmp_fileId_;
    QString tmp_localFileName_;
    QString tmp_remoteName_;
    QByteArray tmp_data_;

    QFile* tmp_file_ = nullptr;
};

#endif // QTONEDRIVE_H
