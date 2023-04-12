#include "AsycReadFile.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/CFCFileApiParse.h"

AsycReadFile::AsycReadFile(int fc, QString path, int offset, int limit, int timeout, QObject *parent)
    : QObject{parent}
{
    m_Stop = false;

    m_fc = fc;
    m_Path = path;

    m_Offset = offset;
    m_Limit = limit;
    m_Timeout = timeout;

    setAutoDelete(true);
}

void AsycReadFile::stopTask()
{
    m_Stop = true;
}

#include <QTextCodec>

void AsycReadFile::run()
{
    m_Stop = false;
    GoStr path(m_Path);
    while(true) {
        if(m_Stop) return;

        QSharedPointer<char> msg(ReadRemoteFileContext(m_fc, path.getGoString(), m_Offset, m_Limit, m_Timeout));

        QByteArray arr = QByteArray::fromBase64(msg.data());

        QString text = QString::fromUtf8(arr.data());

        if(!text.isEmpty()) {
            emit sigText(text);
        }


        if(m_Offset >= m_Limit) m_Stop = true;

        m_Offset += 1024;
    }
}
