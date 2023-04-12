#ifndef ASYCNETSTATUS_H
#define ASYCNETSTATUS_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

//#define ASYCNETSTATUS_DEBUG
#include "ConnectServer/CountDown.h"
#ifdef ASYCNETSTATUS_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

#define ReConnStep 10
#define ReConnMax 30
class AsycNetStatus : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsycNetStatus(int fc, QObject *parent = nullptr);
    void stop();

    void setfc(int fc);

    void slotNoReConn();

    void setCountDown(CountDown *p);
private:
    void run();

signals:
    void sigDelay(QString delay);
    void sigNetworkSpeed(quint64 uploadSpeed, QString uploadSpeedText, quint64 downloadSpeed, QString downloadSpeedText, bool show = true);
    void sigMemoryCapacity(quint64 value, quint64 min, quint64 max, const QString &valueText, const QString &maxText);

    void sigReConnection(int num, bool isExit_C = true, bool valid = true);
private:
    int m_fc;
    bool m_Stop;

    int m_ErrNum;

    int reConnNum;
    bool isReCon;
    CountDown *m_CountDown;


    int num = 0;
};

#endif // ASYCNETSTATUS_H
