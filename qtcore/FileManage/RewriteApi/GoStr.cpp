#include "GoStr.h"

GoStr::GoStr()
{

}

GoStr::GoStr(const QByteArray &str)
{
    setData(str);
}

GoStr::GoStr(const QString &str)
{
    setData(str.toUtf8());
}

GoStr::~GoStr()
{
    if(pStr) delete pStr;
}

void GoStr::setData(const QByteArray &str)
{
    pStr = new QByteArray(str);
    goString.p = pStr->data();
    goString.n = pStr->size();
}

GoString GoStr::getGoString()
{
    return goString;
}
