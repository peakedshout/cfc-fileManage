#include "AsycGetProgress.h"
#include "RewriteApi/GoStr.h"

AsycGetProgress::AsycGetProgress(int fc, QString path, Type type, QObject *parent)
    : QObject{parent}
{
    m_fc= fc;
    m_Path = path;
    m_Stop = false;
    m_Type = type;
    setAutoDelete(false);
}

void AsycGetProgress::setNewTask(QString path, Type type)
{
    m_Path = path;
    m_Type = type;
}

void AsycGetProgress::stop()
{
    m_Stop = true;
}

void AsycGetProgress::run()
{
    m_Stop = false;

    QString p = m_Path;
    if(m_Type == AsycGetProgress::Upload && p.lastIndexOf(".CFCUpload_info") == -1) {
        p = p + ".CFCUpload_info";
    } else if(m_Type == AsycGetProgress::Download && p.lastIndexOf(".CFCDownload_info") == -1) {
        p = p + ".CFCDownload_info";

    }

    GoStr path(p.toUtf8());

    if(m_Type == AsycGetProgress::Download) {
        QFile::remove(m_Path);
    }

    while(true)
    {
        if(m_Stop) return;
        QThread::msleep(100);


        QSharedPointer<char> msg(TransmissionRemoteFileContextToGetProgress(m_fc, path.getGoString()));

        debugMsg(msg.data());

        ProgressContext progressContext;
        CFCFileApiParse::progressContext(msg.data(), progressContext);

        if(progressContext.err.isEmpty()) {
            emit sigDownloadFileProgress(progressContext, false);
        }


        // 下载失败
        if("DownStatusDownloadFailure" == progressContext.status || "UpStatusUploadFailure" == progressContext.status) {
            break;
        }

        // 下载成功
        if((progressContext.type == "TransmissionTypeDown" && QFile::exists(m_Path)) ||
                "DownStatusDownloadCompleted" == progressContext.status ||
                "UpStatusUploadCompleted" == progressContext.status) {

            emit sigDownloadFileProgress(progressContext, true);

            QFile::remove(p);

            debugMsg("加载完成");

            if("UpStatusUploadCompleted" == progressContext.status) {
                emit sigUploadFinished();
            }

            break;
        }


    }

}
