﻿#ifndef ASYCDOWNLOADFILE_H
#define ASYCDOWNLOADFILE_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
//#include <QMessageBox>

//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class AsycDownloadFile : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsycDownloadFile(int fc, QString path, bool existBuff, QObject *parent = nullptr);

    void stopDownloadTask();

private:
    void run();

signals:
    void sigDebug(QString text, QString path);
private:
    int m_fc;
    QString m_Path;
    bool m_ExistBuff;

};

#endif // ASYCDOWNLOADFILE_H
