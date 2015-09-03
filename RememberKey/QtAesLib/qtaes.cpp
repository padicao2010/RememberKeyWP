#include "qtaes.h"

#include <QCryptographicHash>

void QtAes::initialize(const QString &key)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(key.toUtf8());
    QByteArray hashKey = hash.result();

    AES_set_encrypt_key((const unsigned char *)hashKey.data(), 128, &encryptKey);
    AES_set_decrypt_key((const unsigned char *)hashKey.data(), 128, &decryptKey);
}

QString QtAes::encrypt(const QString &input) const
{
    // padding 0
    QByteArray inArray = input.toUtf8();
    int len = inArray.length();
    if(len % AES_BLOCK_SIZE != 0) {
        int pad = AES_BLOCK_SIZE - len % AES_BLOCK_SIZE;
        for(int i =0; i < pad; i++) {
            inArray.append('\0');
        }
        len = inArray.length();
    }

    //  encrypt every block
    QByteArray outArray;
    const unsigned char *inbuffer = (const unsigned char *)inArray.data();
    int remain = len;
    while(remain > 0) {
        encryptBlock(inbuffer, outArray);
        remain -= AES_BLOCK_SIZE;
        inbuffer += AES_BLOCK_SIZE;
    }

    // to base64
    QByteArray base64Array = outArray.toBase64();

    // DONE
    return QString(base64Array);
}

QString QtAes::decrypt(const QString &input) const
{
    // from base64
    QByteArray base64Array;
    base64Array.append(input);
    QByteArray inArray = QByteArray::fromBase64(base64Array);

    // decrypt every block
    QByteArray outArray;
    const unsigned char *inbuffer = (const unsigned char *)inArray.data();
    int remain = inArray.length();
    while(remain > 0) {
        decryptBlock(inbuffer, outArray);
        remain -= AES_BLOCK_SIZE;
        inbuffer += AES_BLOCK_SIZE;
    }

    // DONE
    return QString::fromUtf8(outArray);
}

void QtAes::encryptBlock(const unsigned char *inbuffer, QByteArray &out) const
{
    unsigned char outbuffer[AES_BLOCK_SIZE];
    AES_encrypt(inbuffer, outbuffer, &encryptKey);
    out.append((const char *)outbuffer, AES_BLOCK_SIZE);
}

void QtAes::decryptBlock(const unsigned  char *inbuffer, QByteArray &out) const
{
    unsigned char outbuffer[AES_BLOCK_SIZE];
    AES_decrypt(inbuffer, outbuffer, &decryptKey);
    out.append((const char *)outbuffer, AES_BLOCK_SIZE);
}
