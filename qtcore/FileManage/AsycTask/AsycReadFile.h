#ifndef ASYCREADFILE_H
#define ASYCREADFILE_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

//#define ASYCREADFILE_DEBUG

#ifdef ASYCREADFILE_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class AsycReadFile : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AsycReadFile(int fc, QString path, int offset, int limit, int timeout = 3000, QObject *parent = nullptr);

    void stopTask();

private:
    void run();

signals:
    void sigText(QString text);
private:
    bool m_Stop;

    int m_fc;
    QString m_Path;

    int m_Offset;
    int m_Limit;
    int m_Timeout;

};


#endif // ASYCREADFILE_H
