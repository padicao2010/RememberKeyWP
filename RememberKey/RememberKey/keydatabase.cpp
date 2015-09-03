#include "keydatabase.h"

#include <QDebug>
#include <QCryptographicHash>

#define KEY_PASSWORD_ID 1
#define NONE_QUERY "select * from keypass where id < 0"

#define EXPORT_FILE_TOKEN "RememberKey"

KeyDatabase::KeyDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    queryModel = new QSqlQueryModel();
    cryptoHash = new QCryptographicHash(QCryptographicHash::Sha1);
    cryptoAes = NULL;
}

QSqlQueryModel* KeyDatabase::getQueryModel()
{
    return queryModel;
}

void KeyDatabase::getKeyInQueryModel(int row, KeyInfo *key)
{
    QSqlRecord record = queryModel->record(row);
    decryptRecord(record, key);
}

bool KeyDatabase::decryptRecord(const QSqlRecord &record, KeyInfo *key)
{
    key->setId(record.value("id").toInt());
    key->setName(record.value("name").toString());
    key->setSite(record.value("site").toString());
    const QString &other = record.value("other").toString();
    return setDecryptedOther(other, *key);
}

bool KeyDatabase::decryptQuery(QSqlQuery &query, KeyInfo *key)
{
    while(query.next()) {
        key->setId(query.value("id").toInt());
        key->setName(query.value("name").toString());
        key->setSite(query.value("site").toString());
        const QString &other = query.value("other").toString();
        return setDecryptedOther(other, *key);
    }

    setErrorMessage(QObject::tr("Can't decrypt query"), QObject::tr("No record"));
    return false;
}

void KeyDatabase::forgetPassword()
{
    if(cryptoAes != NULL) {
        cryptoAes = nullptr;
   }
}

bool KeyDatabase::activePassword(const QString &pass, const QtAes *aes)
{
    errorMessage.clear();
    cryptoAes = aes;
    if(checkPassword(pass)) {
        queryModel->setQuery(NONE_QUERY, db);
        lastQuery = NONE_QUERY;
        return true;
    } else {
        forgetPassword();
        return false;
    }
}

bool KeyDatabase::savePassword(const QString &pass)
{
    KeyInfo key;
    key.setId(KEY_PASSWORD_ID);
    key.setName("");

    QString decryptedPass = getCryptoHash(pass);
    key.setPassword(decryptedPass);

    return this->addKeyInfo(key);
}

bool KeyDatabase::checkPassword(const QString &pass)
{
    KeyInfo key;
    if(!getKeyInfo(KEY_PASSWORD_ID, &key)) {
        setErrorMessage(QObject::tr("Check password failed"), errorMessage);
        return false;
    }
    QString decryptedPass = getCryptoHash(pass);
    if(key.getPassword() != decryptedPass) {
        setErrorMessage(QObject::tr("Check password failed"), QObject::tr("password mismatched"));
        return false;
    }
    return true;
}

void KeyDatabase::close()
{
    forgetPassword();
    queryModel->clear();
    if(db.isOpen()) {
        db.close();
    }
}

bool KeyDatabase::create(const QString &path, const QString &password, const QtAes *aes)
{
    errorMessage.clear();
    close();

    filepath = path;
    db.setDatabaseName(path);

    if(!db.open()) {
        setErrorMessage(QObject::tr("Can't create database"), db.lastError().text());
        return false;
    }

    QSqlQuery query(db);
    if(!query.exec(
                "create table keypass ("
                    "id integer primary key, "
                    "name varchar(256) not null, "
                    "site varchar(256), "
                    "other clob not null"
                ")")) {
        setErrorMessage(QObject::tr("Can't create table"), db.lastError().text());
        return false;
    }

    cryptoAes = aes;

    if(!savePassword(password)) {
        setErrorMessage(QObject::tr("Can't save password"), errorMessage);
        return false;
    }

    queryModel->setQuery(NONE_QUERY, db);
    lastQuery = NONE_QUERY;

    return true;
}

bool KeyDatabase::open(const QString &path)
{
    errorMessage.clear();
    close();

    filepath = path;
    db.setDatabaseName(path);

    if(!db.open()) {
        setErrorMessage("Can't open database", db.lastError().text());
        return false;
    }

    return true;
}

bool KeyDatabase::addKeyInfo(const KeyInfo &key)
{
    errorMessage.clear();
    QSqlQuery query(db);
    QString addsql = QString(
        "insert into keypass values (null, \"%1\", \"%2\", \"%3\")")
            .arg(key.getName())
            .arg(key.getSite())
            .arg(getEncryptedOther(key));

    qDebug() << addsql;
    if(!query.exec(addsql)) {
        setErrorMessage("Can't add KeyInfo", query.lastError().text());
        return false;
    }

    return true;
}

bool KeyDatabase::getKeyInfo(int id, KeyInfo *key)
{
    errorMessage.clear();
    QSqlQuery query(db);
    QString getsql = QString(
                "select * from keypass where id = %1").arg(id);
    if(!query.exec(getsql)) {
        setErrorMessage("Can't get KeyInfo", query.lastError().text());
        return false;
    }

    return decryptQuery(query, key);
}

bool KeyDatabase::updateKeyInfo(const KeyInfo &old, const KeyInfo &key)
{
    errorMessage.clear();
    QString update = "";
    bool first = true;
    if(old.getName() != key.getName()) {
        if(!first) {
            update += ", ";
        } else {
            first = false;
        }
        update += QString("name = \"%1\"").arg(key.getName());
    }

    if(old.getSite() != key.getSite()) {
        if(!first) {
            update += ", ";
        } else {
            first = false;
        }
        update += QString("site = \"%1\"").arg(key.getSite());
    }

    if(old.getUsername() != key.getUsername() ||
            old.getPassword() != key.getPassword() ||
            old.getNotes() != key.getNotes()) {
        if(!first) {
            update += ", ";
        } else {
            first = false;
        }
        update += QString("other = \"%1\"")
                .arg(getEncryptedOther(key));
    }

    if(update.isEmpty()) {
        return true;
    }

    QSqlQuery query(db);
    QString updatesql = QString(
                "update keypass set %1 where id = %2")
            .arg(update)
            .arg(key.getId());
    qDebug() << updatesql;
    if(!query.exec(updatesql)) {
        setErrorMessage("Can't update KeyInfo", query.lastError().text());
        return false;
    }

    return true;
}

/*
bool KeyDatabase::addOrUpdateKeyInfo(const KeyInfo &key)
{
    KeyInfo oldKey;
    if(!getKeyInfo(key.getId(), &oldKey)) {
        return addKeyInfo(key);
    } else {
        return updateKeyInfo(oldKey, key);
    }
}
*/

bool KeyDatabase::deleteKeyInfo(int keyId)
{
    errorMessage.clear();
    QSqlQuery query(db);
    QString deletesql = QString(
                "delete from keypass where id = %1").arg(keyId);
    if(!query.exec(deletesql)) {
        setErrorMessage("Can't delete KeyInfo", query.lastError().text());
        return false;
    }

    return true;
}

bool KeyDatabase::search(const QString &searchkey)
{
    errorMessage.clear();
    QString sql;
    if(searchkey.isEmpty()) {
        sql = QString("select * from keypass where name != \"\"");
    } else {
        sql = QString("select * from keypass "
                                 "where name like \"%%1%\" "
                                 "or site like \"%%1%\"")
            .arg(searchkey);
    }
    qDebug() << sql;

    queryModel->setQuery(sql, db);
    lastQuery = sql;

    return true;
}

bool KeyDatabase::reverseKey(int count)
{
    KeyInfo key;

    for(int i = KEY_PASSWORD_ID + 1; i < count; i++) {
        key.setId(i);
        if(!addKeyInfo(key)) {
            setErrorMessage(QObject::tr("Can't reverse key"), errorMessage);
            return false;
        }
    }

    return true;
}

void KeyDatabase::updateQueryModel(const QString &name)
{
    if(name.isEmpty()) {
        queryModel->setQuery(lastQuery, db);
    } else {
        queryModel->setQuery(QString("select * from keypass where name = \"%1\"").arg(name));
    }
}

QString KeyDatabase::getCryptoHash(const QString &source)
{
    QByteArray bytes;
    bytes.append(source);
    cryptoHash->reset();
    cryptoHash->addData(bytes);
    return QString(cryptoHash->result().toBase64());
}

QString KeyDatabase::encryptByAES(const QString &source)
{
    return cryptoAes->encrypt(source);
}

QString KeyDatabase::decryptByAES(const QString &source)
{
    return cryptoAes->decrypt(source);
}

QString KeyDatabase::getEncryptedOther(const KeyInfo &key)
{
    QString source = QString("%1|%2|%3")
            .arg(key.getUsername())
            .arg(key.getPassword())
            .arg(key.getNotes());
    return encryptByAES(source);
}

bool KeyDatabase::setDecryptedOther(const QString &other, KeyInfo &key)
{
    QString decryptedOther = decryptByAES(other);
    const QStringList &strlist = decryptedOther.split("|");
    if(strlist.length() != 3) {
        setErrorMessage(QObject::tr("Can't decrypt data"), QObject::tr("wrong password"));
        return false;
    }
    key.setUsername(strlist.at(0));
    key.setPassword(strlist.at(1));
    key.setNotes(strlist.at(2));
    return true;
}

void KeyDatabase::setErrorMessage(const QString &header, const QString &msg)
{
    errorMessage = QString("%1: { %2}").arg(header).arg(msg);
    qDebug() << errorMessage;
}

bool KeyDatabase::exportToFile(QFile *file)
{
    QSqlQuery query(db);
    if(!query.exec("select * from keypass")) {
        setErrorMessage("Can't export", query.lastError().text());
        return false;
    }
    QString token = cryptoAes->encrypt(cryptoAes->encrypt(EXPORT_FILE_TOKEN)) + "\n";
    file->write(token.toUtf8());
    while(query.next()) {
        QString id = QString::number(query.value("id").toInt());
        QString name = query.value("name").toString();
        QString site = query.value("site").toString();
        QString other = query.value("other").toString();

        QString toWrite = cryptoAes->encrypt(QString("%1|%2|%3|%4")
                .arg(cryptoAes->encrypt(id))
                .arg(cryptoAes->encrypt(name))
                .arg(cryptoAes->encrypt(site))
                .arg(cryptoAes->encrypt(other))) + "\n";
        file->write(toWrite.toUtf8());
    }
    return true;
}

bool KeyDatabase::importFromFile(QFile *file)
{
    qDebug() << "Start Importing";
    QString token = cryptoAes->decrypt(cryptoAes->decrypt(QString::fromUtf8(file->readLine(256))));
    if(!token.startsWith(EXPORT_FILE_TOKEN)) {
        errorMessage = QObject::tr("Unrecognize token, not the proper file or error password");
        return false;
    }

    QSqlQuery query(db);
    while(!file->atEnd()) {
        QString data = cryptoAes->decrypt(QString::fromUtf8(file->readLine()));

        QStringList strlist = data.split("|");
        if(strlist.length() != 4) {
            errorMessage = QObject::tr("Error format line : ") + data;
            return false;
        }

        QString id = cryptoAes->decrypt(strlist.at(0));
        QString name = cryptoAes->decrypt(strlist.at(1));
        QString site = cryptoAes->decrypt(strlist.at(2));
        QString other = cryptoAes->decrypt(strlist.at(3));

        QString getSql = QString("select id from keypass where id = %1").arg(id);
        if(!query.exec(getSql) || !query.next()) {
            // Use add
            QString addSql = QString("insert into keypass values (%1,\"%2\",\"%3\",\"%4\")")
                    .arg(id, name, site, other);
            qDebug() << addSql;
            if(!query.exec(addSql)) {
                setErrorMessage(QObject::tr("Can't add record"), query.lastError().text());
                return false;
            }
        } else {
            // use update
            QString updateSql = QString(
                        "update keypass set name = \"%2\", site = \"%3\","
                        " other = \"%4\" where id = %1"
                        )
                    .arg(id, name, site, other);
            qDebug() << updateSql;
            if(!query.exec(updateSql)) {
                setErrorMessage(QObject::tr("Can't update record"), query.lastError().text());
                return false;
            }
        }
    }

    return true;
}

