#ifndef KEYDATABASE_H
#define KEYDATABASE_H

#include <QtSql>
#include <QCryptographicHash>

#include "keyinfo.h"
#include "../QtAesLib/qtaes.h"

class KeyDatabase
{
public:
    KeyDatabase();

    QSqlQueryModel *getQueryModel();

    void updateQueryModel(const QString &name);
    void getKeyInQueryModel(int row, KeyInfo *key);
    QString getFilePath() { return filepath; }

    bool create(const QString &path, const QString &password, const QtAes *aes);
    bool open(const QString &path);
    bool addKeyInfo(const KeyInfo &key);
    bool updateKeyInfo(const KeyInfo &old, const KeyInfo &key);
    //bool addOrUpdateKeyInfo(const KeyInfo &key);
    bool deleteKeyInfo(int keyId);
    bool getKeyInfo(int id, KeyInfo *key);
    bool search(const QString &searchkey);
    bool reverseKey(int count);

    void forgetPassword();
    bool activePassword(const QString &pass, const QtAes *aes);
    void close();
    bool exportToFile(QFile *file);
    bool importFromFile(QFile *file);

    QString getLastErrorMessage() { return errorMessage; }

private:

    bool decryptRecord(const QSqlRecord &record, KeyInfo *key);
    bool decryptQuery(QSqlQuery &query, KeyInfo *key);
    bool savePassword(const QString &pass);
    bool checkPassword(const QString &pass);
    QString getCryptoHash(const QString &source);
    QString encryptByAES(const QString &source);
    QString decryptByAES(const QString &source);
    QString getEncryptedOther(const KeyInfo &key);
    bool setDecryptedOther(const QString &source, KeyInfo &key);
    void setErrorMessage(const QString &header, const QString &msg);

    QSqlDatabase db;
    QSqlQueryModel *queryModel;
    QCryptographicHash *cryptoHash;
    const QtAes *cryptoAes;
    QString filepath;

    QString errorMessage;

    QString lastQuery;
};

#endif // KEYDATABASE_H
