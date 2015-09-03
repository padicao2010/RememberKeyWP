#ifndef QTAES_H
#define QTAES_H

#include <QObject>
#include <openssl/aes.h>

class QtAes
{
public:
    void initialize(const QString &key);
    QString encrypt(const QString &input) const;
    QString decrypt(const QString &input) const ;

private:
    void encryptBlock(const unsigned char *inbuffer, QByteArray &out) const;
    void decryptBlock(const unsigned char *inbuffer, QByteArray &out) const;

    AES_KEY encryptKey;
    AES_KEY decryptKey;
};

#endif // QTAES_H
