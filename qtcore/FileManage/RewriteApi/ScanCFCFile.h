#ifndef SCANCFCFILE_H
#define SCANCFCFILE_H

#include <QStringList>
#include "GoStr.h"
#include "CFCFileApiParse.h"

class ScanCFCFile {
public:
    enum Type {
        NoType = 0,
        Download,
        Upload
    };

    static QStringList scanCFCFile(int fc, const QString &path, Type type = NoType) {
        GoStr p(path);
        QSharedPointer<char> msg(ScanCFCInfoRemoteFileContext(fc, p.getGoString(), true));

        QList<ScanCFCFileContext> contexts;
        CFCFileApiParse::scanCFCFileContext(msg.data(), contexts);

        QStringList uploadFiles;
        QStringList downloadFiles;
        for(auto &var : contexts) {
            if(var.errMsg.isEmpty()) {
                if(var.downloadPath.isEmpty()) downloadFiles << var.downloadPath;
                if(var.upPath.isEmpty()) uploadFiles << var.upPath;
            }
        }
        if(type == Download) {
            return downloadFiles;
        } else if(type == Upload) {
            return uploadFiles;
        } else {
            downloadFiles.append(uploadFiles);
            return downloadFiles;
        }

    }

};


#endif // SCANCFCFILE_H
