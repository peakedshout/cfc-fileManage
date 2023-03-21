#ifndef CFCFILEAPIPARSE_H
#define CFCFILEAPIPARSE_H

#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class FileContext;
struct NetStatus;
struct ProgressContext;
struct ScanCFCFileContext;

class CFCFileApiParse
{
public:
    CFCFileApiParse();

    static bool initCFCInfo(const QByteArray &json, int &c, QString &errMsg);

    static bool fileContext(const QByteArray &json, FileContext &context);

    static void netStatusContext(const QByteArray &json, NetStatus &netStatus);

    static bool fileContextToNewTask(const QByteArray &json, QString &path, QString &errMsg);

    static bool progressContext(const QByteArray &json, ProgressContext &progressContext);

    static bool scanCFCFileContext(const QByteArray &json, QList<ScanCFCFileContext> &contexts);
};

class FileContext {
public:
    FileContext() {
        clear();
    }

    struct File
    {
        QString name;
        QString size;
        QString mode;
        QString modTime;
        bool isDir;
        void clear() {
            name = "";
            size = "";
            mode = "";
            modTime = "";
            isDir = false;
        }
    };

    struct Config
    {
        QString deviceName;
        QString deviceType;
        quint64 rootSize;
        quint64 maxRootSize;
        QString rootSizeErr;
        QList<QString> permissionList;
        void clear() {
            deviceName = "";
            deviceType = "";
            rootSize = 0;
            maxRootSize = 0;
            permissionList.clear();
        }
    };

    void clear() {
        id = -1;
        current = "";
        fileList.clear();
        config.clear();
        error = "";
    }
public:
    int id;
    QString current;
    QList<File> fileList;
    Config config;
    QString error;
};

struct NetStatus{
    quint64 rootSize=0;
    quint64 maxRootSize=0;
    QString rootSizeShow="";
    QString maxRootSizeShow="";
    QString rootSizeErr="";

    QString delay="";
    QString delayErr="";

    quint64 uploadSpeed=0;
    quint64 downloadSpeed=0;
    QString uploadSpeedShow="";
    QString downloadSpeedShow="";
};

struct ProgressContext
{
    quint64 currProgress = 0;
    quint64 total = 0;
    QString name = "";
    QString status = "";
    QString type = "";
    QString from = "";
    QString to = "";

    quint64 uploadSpeed = 0;
    quint64 downloadSpeed = 0;
    QString uploadSpeedShow = "";
    QString downloadSpeedShow = "";

    QString err = "";
};

struct ScanCFCFileContext
{
    QString downloadPath = "";
    QString upPath = "";
    QString pathErr = "";

    QString errMsg = "";
};
#endif // CFCFILEAPIPARSE_H
