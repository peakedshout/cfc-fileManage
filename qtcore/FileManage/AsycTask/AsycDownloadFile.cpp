#include "AsycDownloadFile.h"
#include "RewriteApi/GoStr.h"

AsycDownloadFile::AsycDownloadFile(int fc, QString path, bool existBuff, QObject *parent)
    : QObject{parent}
{
    m_fc = fc;

    if(path.lastIndexOf(".CFCDownload_info") == -1) {
        path = path + ".CFCDownload_info";
    }

    debugMsg(path);
    m_Path = path;
    m_ExistBuff = existBuff;
    setAutoDelete(false);
}

void AsycDownloadFile::stopDownloadTask()
{
    GoStr path(m_Path.toUtf8());
    TransmissionFileRemoteFileContextToStop(m_fc, path.getGoString());
}

void AsycDownloadFile::run()
{
    GoStr path(m_Path.toUtf8());
    QSharedPointer<char> msg(DownRemoteFileContextToStartDown(m_fc, path.getGoString(), !m_ExistBuff));

    m_ExistBuff = true;
    if(QString(msg.data()).contains("serverName is inconsistency") || QString(msg.data()).contains("userName is inconsistency")) {
        emit sigDebug(QString("%1任务无法导入").arg(m_Path), m_Path);
    }

    debugMsg(msg.data());

}
