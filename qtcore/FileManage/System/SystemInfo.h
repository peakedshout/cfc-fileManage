#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QProcess>
#include <QString>
#include <QDebug>

class SystemInfo
{
public:
    static QString getInfo(const QString &cmd);
    static QString getBaseboardUuid();
};

#endif // SYSTEMINFO_H
