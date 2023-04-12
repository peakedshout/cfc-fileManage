#include "CFCFileApiParse.h"

CFCFileApiParse::CFCFileApiParse()
{
}

bool CFCFileApiParse::initCFCInfo(const QByteArray &json, int &c, QString &errMsg)
{
    if(json.isEmpty()) {
        errMsg = "Json数据为空";
        return false;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        errMsg = "JsonData Err!";
        return false;
    }
    if(!jsonDoc.isObject()) {
        errMsg = "It not is JsonObject";
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if(jsonObj.contains("ErrMsg")) {
        errMsg = jsonObj.value("ErrMsg").toString();
        if(!errMsg.isEmpty()) {
            return false;
        }
    }

    if(jsonObj.contains("ClientContext")) {
        c = jsonObj.value("ClientContext").toInt();
        return true;
    }

    return false;
}

bool CFCFileApiParse::fileContext(const QByteArray &json, FileContext &context)
{
    QJsonParseError error;
    QJsonDocument JsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        context.error = error.errorString();
        return false;
    }

    if(!JsonDoc.isObject()) {
        context.error = "It not is JsoonObject";
        return false;
    }

    QJsonObject jsonObj = JsonDoc.object();

    if(jsonObj.contains("Err")) {
        QString err = jsonObj.value("Err").toString();
        if(!err.isEmpty()) {
            context.error = err;
            return false;
        }
    }

    if(jsonObj.contains("Id"))
        context.id = jsonObj.value("Id").toInt();

    if(jsonObj.contains("Current"))
        context.current = jsonObj.value("Current").toString();

    if(jsonObj.contains("Config") && jsonObj.value("Config").isObject()) {
        QJsonObject configObj = jsonObj.value("Config").toObject();

        if(configObj.contains("DeviceName"))
            context.config.deviceName = configObj.value("DeviceName").toString();

        if(configObj.contains("DeviceType"))
            context.config.deviceType = configObj.value("DeviceType").toString();

        if(configObj.contains("RootSize"))
            context.config.rootSize = configObj.value("RootSize").toVariant().toLongLong();

        if(configObj.contains("MaxRootSize"))
            context.config.maxRootSize = configObj.value("MaxRootSize").toVariant().toLongLong();

        if(configObj.contains("RootSizeErr"))
            context.config.rootSizeErr = configObj.value("RootSizeErr").toString();

        if(configObj.contains("PermissionList") && configObj.value("PermissionList").isArray()) {
            QJsonArray permissionList = configObj.value("PermissionList").toArray();
            for(auto json : permissionList) {
                context.config.permissionList.push_back(json.toString());
            }
        }
    }

    if(jsonObj.contains("FileList") && jsonObj.value("FileList").isArray()) {
        QJsonArray jsonArr = jsonObj.value("FileList").toArray();
        for(auto json : jsonArr) {
            if(json.isObject()) {
                QJsonObject obj = json.toObject();
                FileContext::File file;

                if(obj.contains("Name"))
                    file.name = obj.value("Name").toString();

                if(obj.contains("Size"))
                    file.size = obj.value("Size").toString();

                if(obj.contains("Mode"))
                    file.mode = obj.value("Mode").toString();

                if(obj.contains("ModTime"))
                    file.modTime = obj.value("ModTime").toString();

                if(obj.contains("IsDir"))
                    file.isDir = obj.value("IsDir").toBool();

                context.fileList.push_back(file);
            }
        }
    }

    return true;
}

void CFCFileApiParse::netStatusContext(const QByteArray &json, NetStatus &netStatus)
{
//{"RootSize":1238549443,"MaxRootSize":12385275420,"RootSizeShow":"1.15GB","MaxRootSizeShow":"11.53GB",
//"RootSizeErr":"","Delay":"58μs","DelayErr":"","NetworkSpeed":{"UploadSpeed":1818,"DownloadSpeed":40686982,
//"UploadSpeedShow":"1.78KB/s","DownloadSpeedShow":"38.80MB/s"}}

    QJsonParseError error;
    QJsonDocument JsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject jsonObj = JsonDoc.object();

    if(jsonObj.contains("RootSizeErr")) {
        netStatus.rootSizeErr = jsonObj.value("RootSizeErr").toString();
        if(netStatus.rootSizeErr.isEmpty()) {
            if(jsonObj.contains("RootSize")) {
                netStatus.rootSize = jsonObj.value("RootSize").toVariant().toLongLong();
            }
            if(jsonObj.contains("MaxRootSize")) {
                netStatus.maxRootSize = jsonObj.value("MaxRootSize").toVariant().toLongLong();
            }
            if(jsonObj.contains("RootSizeShow")) {
                netStatus.rootSizeShow = jsonObj.value("RootSizeShow").toString();
            }
            if(jsonObj.contains("MaxRootSizeShow")) {
                netStatus.maxRootSizeShow = jsonObj.value("MaxRootSizeShow").toString();
            }

        }
    }

    if(jsonObj.contains("DelayErr")) {
        netStatus.delayErr = jsonObj.value("DelayErr").toString();
        if(netStatus.delayErr.isEmpty()) {
            if(jsonObj.contains("Delay")) {
                netStatus.delay = jsonObj.value("Delay").toString();
            }
        }
    }

    if(jsonObj.contains("NetworkSpeed") && jsonObj.value("NetworkSpeed").isObject()) {
        QJsonObject obj = jsonObj.value("NetworkSpeed").toObject();

        if(obj.contains("UploadSpeed")) {
            netStatus.uploadSpeed = obj.value("UploadSpeed").toVariant().toLongLong();
        }
        if(obj.contains("DownloadSpeed")) {
            netStatus.downloadSpeed = obj.value("DownloadSpeed").toVariant().toLongLong();
        }

        if(obj.contains("UploadSpeedShow")) {
            netStatus.uploadSpeedShow = obj.value("UploadSpeedShow").toString();
        }
        if(obj.contains("DownloadSpeedShow")) {
            netStatus.downloadSpeedShow = obj.value("DownloadSpeedShow").toString();
        }

    }

    return;
}

bool CFCFileApiParse::fileContextToNewTask(const QByteArray &json, QString &path, QString &errMsg)
{
    if(json.isEmpty()) {
        errMsg = "Json数据为空";
        return false;
    }

    QJsonParseError error;
    QJsonDocument JsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        errMsg = error.errorString();
        return false;
    }

    QJsonObject jsonObj = JsonDoc.object();
    if(jsonObj.contains("ErrMsg")) {
        errMsg = jsonObj.value("ErrMsg").toString();
        if(!errMsg.isEmpty()) return false;
    }

    if(jsonObj.contains("InfoPath")) {
        path = jsonObj.value("InfoPath").toString();
        return true;
    }

    return false;
}

bool CFCFileApiParse::progressContext(const QByteArray &json, ProgressContext &progressContext)
{
    /*{"Info":{"FileName":"文件管理.png","Type":"TransmissionTypeDown","From":"/文件管理.png",
    "To":"E:/Qt_Qbject/Qt_FileManage/bin_FileManage_output/debug/DownloadFileDir/文件管理.png",
    "Progress":0,"Total":1,"Status":"DownStatusDownloadReset",
    "NetworkSpeed":{"UploadSpeed":0,"DownloadSpeed":0,"UploadSpeedShow":"0.00B/s","DownloadSpeedShow":"0.00B/s"}},"ErrMsg":""}
    */
    QJsonParseError error;
    QJsonDocument JsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        progressContext.err = error.errorString();
        return false;
    }

    if(!JsonDoc.isObject()) {
        progressContext.err = "It not is JsonObject";
        return false;
    }

    QJsonObject jsonObj = JsonDoc.object();

    if(jsonObj.contains("ErrMsg")) {
        progressContext.err = jsonObj.value("ErrMsg").toString();
        if(!progressContext.err.isEmpty()) {
            return false;
        }
    }

    if(jsonObj.contains("Info") && jsonObj.value("Info").isObject()) {
        QJsonObject infoObj = jsonObj.value("Info").toObject();
        if(infoObj.contains("Status")) {
            progressContext.status = infoObj.value("Status").toString();
        }
        if(infoObj.contains("Progress")) {
            progressContext.currProgress = infoObj.value("Progress").toVariant().toULongLong();
        }
        if(infoObj.contains("Total")) {
            progressContext.total = infoObj.value("Total").toVariant().toULongLong();
        }
        if(infoObj.contains("FileName")) {
            progressContext.name = infoObj.value("FileName").toString();
        }
        if(infoObj.contains("Type")) {
            progressContext.type = infoObj.value("Type").toString();
        }
        if(infoObj.contains("From")) {
            progressContext.from = infoObj.value("From").toString();
        }
        if(infoObj.contains("To")) {
            progressContext.to = infoObj.value("To").toString();
        }

        if(infoObj.contains("NetworkSpeed") && infoObj.value("NetworkSpeed").isObject()) {
            QJsonObject obj = infoObj.value("NetworkSpeed").toObject();
            if(obj.contains("UploadSpeed")) {
                progressContext.uploadSpeed = obj.value("UploadSpeed").toVariant().toULongLong();
            }
            if(obj.contains("DownloadSpeed")) {
                progressContext.downloadSpeed = obj.value("DownloadSpeed").toVariant().toULongLong();
            }
            if(obj.contains("UploadSpeedShow")) {
                progressContext.uploadSpeedShow = obj.value("UploadSpeedShow").toString();
            }
            if(obj.contains("DownloadSpeedShow")) {
                progressContext.downloadSpeedShow = obj.value("DownloadSpeedShow").toString();
            }
        }
    }

    return true;
}

bool CFCFileApiParse::scanCFCFileContext(const QByteArray &json, QList<ScanCFCFileContext> &contexts)
{
   // {"Data":[{"DownInfoPath":"E:\\Qt_Qbject\\Qt_FileManage\\bin_FileManage_output\\debug\\DownloadFileDir\\qt-opensource-windows-x86-mingw492-5.6.1-1.exe.CFCDownload_info","UpInfoPath":"","ErrMsg":""}],"ErrMsg":""}
    QJsonParseError error;
    QJsonDocument JsonDoc = QJsonDocument::fromJson(json, &error);
    if(error.error != QJsonParseError::NoError) {
        return false;
    }

    if(!JsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = JsonDoc.object();

    if(jsonObj.contains("ErrMsg")) {
        QString errMsg = jsonObj.value("ErrMsg").toString();
        if(!errMsg.isEmpty()) {
            return false;
        }
    }

    if(jsonObj.contains("Data") && jsonObj.value("Data").isArray()) {
        QJsonArray arr = jsonObj.value("Data").toArray();
        for(auto info : arr) {
            if(info.isObject()) {
                QJsonObject infoObj = info.toObject();
                ScanCFCFileContext context;

                if(infoObj.contains("ErrMsg")) {
                    context.pathErr = infoObj.value("ErrMsg").toString();
                    if(!context.pathErr.isEmpty()) {
                        return false;
                    }
                }

                if(infoObj.contains("DownInfoPath")) {
                    context.downloadPath = infoObj.value("DownInfoPath").toString();
                }

                if(infoObj.contains("UpInfoPath")) {
                    context.upPath = infoObj.value("UpInfoPath").toString();
                }

                contexts.push_back(context);
            }
        }
    }

    return true;
}









