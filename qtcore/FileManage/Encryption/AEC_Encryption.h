#ifndef AEC_ENCRYPTION_H
#define AEC_ENCRYPTION_H

#include <QByteArray>
#include <QCryptographicHash>

class AEC_Encryption
{
public:
    static QByteArray getKey(const QByteArray &key);
    static QByteArray encode(const QByteArray &encodedText, const QByteArray &key);
    static QByteArray decode(const QByteArray &decodeText, const QByteArray &key);
};

#endif // AEC_ENCRYPTION_H
