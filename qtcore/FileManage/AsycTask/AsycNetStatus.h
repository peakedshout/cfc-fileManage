#ifndef ASYCNETSTATUS_H
#define ASYCNETSTATUS_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class AsycNetStatus : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsycNetStatus(int fc, QObject *parent = nullptr);
    void stop();
private:
    void run();

signals:
    void sigDelay(QString delay);
    void sigNetworkSpeed(quint64 uploadSpeed, QString uploadSpeedText, quint64 downloadSpeed, QString downloadSpeedText);
    void sigMemoryCapacity(quint64 value, quint64 min, quint64 max, const QString &valueText, const QString &maxText);

private:
    int m_fc;
    bool m_Stop;
};

#endif // ASYCNETSTATUS_H
