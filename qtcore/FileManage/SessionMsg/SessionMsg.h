#ifndef SESSIONMSG_H
#define SESSIONMSG_H

#include <QString>
#include <QMap>
#include <QFile>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>


//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class SessionMsg
{
public:
    SessionMsg(const QByteArray &key);

    void readSessionMsg(const QString &path = "config.ini");
    void wirteSessionMsg(const QString &path = "config.ini");
public:
    struct SessionInfo {
        QString clientName;
        QString serverName;
        QString ip;
        int port = 0;

        int logLevel = 0;
        bool useStack = false;
        int logPort = 9888;

        int sessions = 5;
        int uploadNum = 3;
        int downloadNum = 3;
        QString key;
        QString userKey;
        QString localPath;
        QString scanPath;
        QString savePath;
    };

    bool equality();
    void recover();
    void save();
public:
    QString prove;
    int width;
    int height;
    QMap<QString, SessionInfo> sessions;

private:
    QByteArray m_Key;

    QMap<QString, SessionInfo> m_Sessions;
};

#endif // SESSIONMSG_H
