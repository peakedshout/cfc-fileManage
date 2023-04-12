#include "SessionMsg.h"
#include "Encryption/AEC_Encryption.h"
#include "System/SystemInfo.h"

SessionMsg::SessionMsg(const QByteArray &key)
{
    m_Key = key;
}

void SessionMsg::readSessionMsg(const QString &path)
{
    QByteArray json;
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        QDataStream out(&file);
        out >> json;

        file.close();
    }

    // 解密
    json = AEC_Encryption::decode(json, AEC_Encryption::getKey(m_Key));

    QJsonDocument doc = QJsonDocument::fromJson(QString(json).toUtf8());

    if(!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();

        if(obj.contains("prove")) {
            prove = obj.value("prove").toString();
        }

        if(obj.contains("width")) {
            width = obj.value("width").toInt();
        }

        if(obj.contains("height")) {
            height = obj.value("height").toInt();
        }

        if(obj.contains("sessions") && obj.value("sessions").isArray()) {
            QJsonArray arr = obj.value("sessions").toArray();

            for(const auto &var : arr) {
                if(!var.isObject()) continue;

                QJsonObject &&infoObj = var.toObject();

                SessionInfo info;

                if(infoObj.contains("clientName")) {
                    info.clientName = infoObj.value("clientName").toString();
                }


                if(infoObj.contains("serverName")) {
                    info.serverName = infoObj.value("serverName").toString();
                }

                if(infoObj.contains("ip")) {
                    info.ip = infoObj.value("ip").toString();
                }

                if(infoObj.contains("port")) {
                    info.port = infoObj.value("port").toInt();
                }

                if(infoObj.contains("logLevel")) {
                    info.logLevel = infoObj.value("logLevel").toInt();
                }

                if(infoObj.contains("useStack")) {
                    info.useStack = infoObj.value("useStack").toBool();
                }

                if(infoObj.contains("logPort")) {
                    info.logPort = infoObj.value("logPort").toInt();
                }

                if(infoObj.contains("sessions")) {
                    info.sessions = infoObj.value("sessions").toInt();
                }

                if(infoObj.contains("uploadNum")) {
                    info.uploadNum = infoObj.value("uploadNum").toInt();
                }

                if(infoObj.contains("downloadNum")) {
                    info.downloadNum = infoObj.value("downloadNum").toInt();
                }

                if(infoObj.contains("key")) {
                    info.key = infoObj.value("key").toString();
                }

                if(infoObj.contains("fileKey")) {
                    info.userKey = infoObj.value("fileKey").toString();
                }

                if(infoObj.contains("localPath")) {
                    info.localPath = infoObj.value("localPath").toString();
                }

                if(infoObj.contains("scanPath")) {
                    info.scanPath = infoObj.value("scanPath").toString();
                }

                if(infoObj.contains("savePath")) {
                    info.savePath = infoObj.value("savePath").toString();
                }

                sessions.insert(info.serverName + "-" + info.clientName, info);
            }

        }

    }

    if(prove.isEmpty()) {
        prove = SystemInfo::getBaseboardUuid();
    }

    m_Sessions = sessions;
}

void SessionMsg::wirteSessionMsg(const QString &path)
{    
    QJsonObject jsonObj;
    jsonObj.insert("prove", prove);
    jsonObj.insert("width", width);
    jsonObj.insert("height", height);

    QJsonArray jsonArr;
    int i = 0;
    QList list = sessions.values();
    for(auto &var : list) {
        QJsonObject obj;
        obj.insert("clientName", var.clientName);
        obj.insert("serverName", var.serverName);
        obj.insert("ip", var.ip);
        obj.insert("port", var.port);

        obj.insert("logLevel", var.logLevel);
        obj.insert("useStack", var.useStack);
        obj.insert("logPort", var.logPort);

        obj.insert("sessions", var.sessions);
        obj.insert("uploadNum", var.uploadNum);
        obj.insert("downloadNum", var.downloadNum);

        obj.insert("key", var.key);
        obj.insert("fileKey", var.userKey);
        obj.insert("localPath", var.localPath);
        obj.insert("scanPath", var.scanPath);
        obj.insert("savePath", var.savePath);

        jsonArr.insert(i, obj);
        i++;
    }

    jsonObj.insert("sessions", jsonArr);

    QJsonDocument doc;
    doc.setObject(jsonObj);

    QByteArray json = doc.toJson(QJsonDocument::Compact);

    // 加密
    json = AEC_Encryption::encode(json, AEC_Encryption::getKey(m_Key));

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)) return;
    QDataStream in(&file);

    in << json;

    file.close();
}

bool SessionMsg::equality()
{
    QList values = m_Sessions.values();

    QList values1 = sessions.values();
    if(values.size() != values1.size()) return false;

    int count = values.size();
    for(int i = 0; i < count; i++) {
        if(values1.at(i).clientName != values.at(i).clientName)
            return false;

        if(values1.at(i).serverName != values.at(i).serverName)
            return false;

        if(values1.at(i).userKey != values.at(i).userKey)
            return false;

        if(values1.at(i).key != values.at(i).key)
            return false;

        if(values1.at(i).ip != values.at(i).ip)
            return false;

        if(values1.at(i).port != values.at(i).port)
            return false;

        if(values1.at(i).logLevel != values.at(i).logLevel)
            return false;

        if(values1.at(i).useStack != values.at(i).useStack)
            return false;

        if(values1.at(i).logPort != values.at(i).logPort)
            return false;

        if(values1.at(i).sessions != values.at(i).sessions)
            return false;

        if(values1.at(i).downloadNum != values.at(i).downloadNum)
            return false;

        if(values1.at(i).uploadNum != values.at(i).uploadNum)
            return false;

        if(values1.at(i).localPath != values.at(i).localPath)
            return false;

        if(values1.at(i).savePath != values.at(i).savePath)
            return false;

        if(values1.at(i).scanPath != values.at(i).scanPath)
            return false;
    }

    return true;

}

void SessionMsg::recover()
{
    sessions.clear();
    sessions = m_Sessions;
}

void SessionMsg::save()
{
    m_Sessions.clear();
    m_Sessions = sessions;
}
