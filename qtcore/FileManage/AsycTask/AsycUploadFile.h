#ifndef ASYCUPLOADFILE_H
#define ASYCUPLOADFILE_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

//#define ASYCUPLOADFILE_DEBUG

#ifdef ASYCUPLOADFILE_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class AsycUploadFile : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsycUploadFile(int fc, QString path, bool existBuff, QObject *parent = nullptr);

    void stopUploadTask();

private:
    void run();

signals:
    void sigDebug(QString text, QString path);
private:
    int m_fc;
    QString m_Path;
    bool m_ExistBuff;

};

#endif // ASYCUPLOADFILE_H
