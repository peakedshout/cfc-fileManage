#include "SystemInfo.h"

QString SystemInfo::getInfo(const QString &cmd)
{
    QProcess p;				//启动外部程序
    p.start(cmd);			//一体式启动，不分离，主程序退出则启动程序退出,使用close关闭
    //p.startDetached(cmd)	//分离式启动，主程序退出后，外部程序继续运行。
    p.waitForFinished(-1);	//超时等待，设置为-1，直到执行完成。
    QString result = QString::fromLocal8Bit(p.readAllStandardOutput());
    QStringList list = cmd.split(" ");
    result = result.remove(list.last(), Qt::CaseInsensitive);
    result = result.replace("\r", "");
    result = result.replace("\n", "");
    result = result.simplified();
    p.close();
    return result;
}

//查询主板唯一标识
QString SystemInfo::getBaseboardUuid()
{
    QString version= QSysInfo::productType();
    if(version == "windows" || version == "winrt") {
        return getInfo("wmic csproduct get uuid");
    } else if(version == "android") {
        return getInfo("sudo cat /sys/class/dmi/id/product_uuid");
    } else if(version == "osx" || version == "macos" || version == "ios" || version == "tvOS" || version == "watchOS") {
        return getInfo("system_profiler SPHardwareDataType | grep UUID");
    }
    return "";
}

