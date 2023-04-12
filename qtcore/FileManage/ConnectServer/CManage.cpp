#include "CManage.h"

CManage::CManage()
{

}

void CManage::removeFc(int fc)
{
    if(fcInfo.contains(fc)) {
        int c = fcInfo.value(fc).c;
        if(C_FCsMap.contains(c)) {
            QList<int> l = C_FCsMap.value(c);
            l.removeOne(fc);
            CloseFileContext(fc);
            C_FCsMap.insert(c, l);
        }
        fcInfo.remove(fc);
    }
}

void CManage::closeCBy0()
{
    QList keys = C_FCsMap.keys();
    QList<int> list;
    foreach (auto key, keys) {
        QList<int> fcs = C_FCsMap.value(key);
        if(fcs.size() == 0) {
            Close(key);

            list << key;
        }
    }

    foreach(auto key, list) {
        C_FCsMap.remove(key);
        QList<int> fcs = C_FCsMap.value(key);
        for(int i = 0; i < fcs.size(); ++i) {
            if(fcInfo.contains(fcs.at(i))) {
                fcInfo.remove(fcs.at(i));
            }
        }
    }


}

bool CManage::findC(const QString &ip, int &c)
{
    QList keys = fcInfo.keys();

    foreach (auto key, keys) {
        auto var = fcInfo.value(key);
        if(var.ip == ip) {
            c = var.c;
            return true;
        }
    }

    return false;
}

