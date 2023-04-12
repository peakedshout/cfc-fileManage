#ifndef ASYCGETPROGRESS_H
#define ASYCGETPROGRESS_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QFileInfo>
#include "RewriteApi/CFCFileApiParse.h"

//#define ASYCGETPROGRESS_DEBUG

#ifdef ASYCGETPROGRESS_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class AsycGetProgress : public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum Type {
        Download = 0,
        Upload
    };

public:
    explicit AsycGetProgress(int fc, QString path, AsycGetProgress::Type type, QObject *parent = nullptr);

    void setNewTask(QString path, AsycGetProgress::Type type);
    void stop();

private:
    void run();

signals:
    void sigDownloadFileProgress(ProgressContext progressContext, bool finish);

    void sigUploadFinished();
private:
    int m_fc;
    QString m_Path;
    bool m_Stop;
    AsycGetProgress::Type m_Type;
};

#endif // ASYCGETPROGRESS_H
