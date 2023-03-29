#include "AEC_Encryption.h"
#include "qaesencryption.h"

QByteArray AEC_Encryption::getKey(const QByteArray &key)
{
    return QCryptographicHash::hash(key, QCryptographicHash::Md5);
}

QByteArray AEC_Encryption::encode(const QByteArray &encodedText, const QByteArray &key)
{
    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB, QAESEncryption::ZERO);
    return encryption.encode(encodedText, key).toHex();
}

QByteArray AEC_Encryption::decode(const QByteArray &decodeText, const QByteArray &key)
{
    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB, QAESEncryption::ZERO);
    return encryption.decode(QByteArray::fromHex(decodeText), key);
}
