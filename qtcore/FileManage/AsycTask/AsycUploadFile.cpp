#include "AsycUploadFile.h"
#include "RewriteApi/GoStr.h"

AsycUploadFile::AsycUploadFile(int fc, QString path, bool existBuff, QObject *parent): QObject{parent}
{
    m_fc = fc;

    if(path.lastIndexOf(".CFCUpload_info") == -1) {
        path = path + ".CFCUpload_info";
    }

    m_Path = path;
    m_ExistBuff = existBuff;
    setAutoDelete(false);
}


void AsycUploadFile::stopUploadTask()
{
    GoStr path(m_Path.toUtf8());
    TransmissionFileRemoteFileContextToStop(m_fc, path.getGoString());
}

void AsycUploadFile::run()
{ 
    GoStr path(m_Path.toUtf8());
    QSharedPointer<char> msg(UpRemoteFileContextToStartUp(m_fc, path.getGoString(), !m_ExistBuff));

    debugMsg(msg.data());
}
