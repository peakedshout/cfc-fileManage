#ifndef GOSTR_H
#define GOSTR_H

#include <QByteArray>
#include <QSharedPointer>
#include "cfcfile_api_win_amd64.h"

class GoStr
{
public:
    GoStr();
    GoStr(const QByteArray &str);
    GoStr(const QString &str);
    ~GoStr();
    void setData(const QByteArray &str);
    GoString getGoString();
private:
    QByteArray* pStr = nullptr;
    GoString goString;
};

#endif // GOSTR_H
