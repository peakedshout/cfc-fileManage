#ifndef CMANAGE_H
#define CMANAGE_H

#include <QList>
#include <QMap>
#include "RewriteApi/cfcfile_api_win_amd64.h"

struct SessionInfo
{
    int c = -1;
    int fc = -1;
    QString ip;
    QString serName;
    QString cliName;
    QString key;
    QString filekey;
    QString dlName;
};

class CManage
{
public:
    CManage();

    void removeFc(int fc);

    void closeCBy0();

    bool findC(const QString &ip, int &c);

    QMap<int, SessionInfo> fcInfo;
    QMap<int, QList<int>> C_FCsMap;

    QMap<int, QString> C_DLMap;
};

#endif // CMANAGE_H
