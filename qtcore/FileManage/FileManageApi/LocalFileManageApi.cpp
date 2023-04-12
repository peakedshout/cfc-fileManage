#include "LocalFileManageApi.h"

LocalFileManageApi::LocalFileManageApi()
{

}

LocalFileManageApi::~LocalFileManageApi()
{

}

bool LocalFileManageApi::open(const QString &path)
{
    QFileInfo fileInfo(path);
    if(fileInfo.isFile()) {
        return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    } else if(fileInfo.isDir()) {

    }

    return false;
}

bool LocalFileManageApi::copy(const QString &oldPath, const QString &newPath)
{
    if(!QFile::exists(oldPath) || oldPath == newPath) return false;
    QFileInfo oldFileInfo(oldPath);

    QMessageBox::StandardButton state = QMessageBox::No;

    if(oldFileInfo.isFile()) {
        return copyFile(oldPath, newPath, state);
    } else if(oldFileInfo.isDir()) {
        return copyDir(oldPath, newPath, state);
    }

    return false;
}

bool LocalFileManageApi::copyFile(const QString &oldPath, const QString &newPath, QMessageBox::StandardButton &state)
{

    QFileInfo newFileInfo(newPath);

    QDir dir;
    dir.mkpath(newFileInfo.dir().absolutePath());


    if(newFileInfo.exists() && state != QMessageBox::YesToAll) {
        if(state == QMessageBox::NoToAll) return false;
        int btn = QMessageBox::information(nullptr, "文件存在", "文件已存在，是否覆盖", QMessageBox::YesToAll |
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll, QMessageBox::Yes);

        if(btn == QMessageBox::No) return false;
        else if(btn == QMessageBox::YesToAll) state = QMessageBox::YesToAll;
        else if(btn == QMessageBox::NoToAll) {
            state = QMessageBox::NoToAll;
            return false;
        }
    }

    QFile::remove(newPath);
    return QFile::copy(oldPath, newPath);
}

bool LocalFileManageApi::copyDir(const QString &oldPath, const QString &newPath, QMessageBox::StandardButton &state)
{
    if(oldPath.isEmpty()) return false;

    QFileInfoList fileInfoList = QDir(oldPath).entryInfoList();
    QDir destDir(newPath);
    destDir.mkpath(newPath);
    foreach(const QFileInfo& fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

        if(fileInfo.isFile()) {
            copyFile(fileInfo.filePath(), destDir.filePath(fileInfo.fileName()), state);
        } else if(fileInfo.isDir()) {
            copyDir(fileInfo.filePath(), destDir.filePath(fileInfo.fileName()), state);
        }
    }
    return true;
}

// 删除文件
bool LocalFileManageApi::remove(const QString &path)
{
    QFileInfo sourceFileInfo(path);
    if(!sourceFileInfo.exists()) return false;
    if(sourceFileInfo.isFile()) {
        return sourceFileInfo.dir().remove(sourceFileInfo.fileName());
    } else if(sourceFileInfo.isDir()) {
        QDir sourceDir(path);
        QFileInfoList fileInfoList = sourceDir.entryInfoList(); // 获取所有的文件信息
        foreach (QFileInfo fileInfo, fileInfoList)
        {
            if(fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;
            remove(fileInfo.absoluteFilePath());
        }
        sourceDir.rmpath(sourceDir.absolutePath()); // 这时候文件夹已经空了，再删除文件夹本身
        return true;
    }
    return false;
}


bool LocalFileManageApi::move(const QString &oldPath, const QString &newPath)
{
    if(oldPath == newPath) return false;
    if(QFile::exists(newPath)) {
        int btn = QMessageBox::information(nullptr, "文件存在", "文件已存在，是否覆盖", QMessageBox::Yes | QMessageBox::No , QMessageBox::Yes);
        if(btn == QMessageBox::No) return false;
    }
    QFile::remove(newPath);
    return QFile::rename(oldPath, newPath);
}

void LocalFileManageApi::readerDir(const QString &path, QStandardItemModel *model)
{
    if(!model) return;
    model->setRowCount(0);
    QFileInfo fileInfo(path);
    if(!fileInfo.exists()) {
        QMessageBox::critical(nullptr, "路径信息", "路径错误，路径不存在");
        return;
    }


    if(fileInfo.isDir()) {
        QStandardItem *item = nullptr;
        QFileInfoList fileInfoList = QDir(path).entryInfoList();
        int count = -1;
        for(const QFileInfo& info : fileInfoList) {
//            if(info.fileName() == "" || info.fileName() == "." || info.fileName() == "..") continue;

            if(info.fileName() == "" || info.fileName() == ".") continue;

            count++;
            if(info.fileName() == "..") {
                item = new QStandardItem(QIcon(":/png/dir.png"), info.fileName());
                model->setItem(count, 0, item);
                item = new QStandardItem();
                model->setItem(count, 1, item);
                item = new QStandardItem();
                model->setItem(count, 2, item);
                item = new QStandardItem();
                model->setItem(count, 3, item);
                continue;
            }

            QFileInfo fileInfo(info.fileName());
            QString suffix = fileInfo.suffix();
            QString fileIconStr = QString(":/png/%1.png").arg(suffix);
            fileInfo.setFile(fileIconStr);
            if(!fileInfo.exists()) {
                fileIconStr = ":/png/what.png";
            }

            item = new QStandardItem(QIcon(info.isDir()? ":/png/dir.png" : fileIconStr), info.fileName());
            model->setItem(count, 0, item);

            item = new QStandardItem(info.lastModified().toString("yyyy/MM/dd hh:mm"));
            model->setItem(count, 1, item);

            item = new QStandardItem(info.isDir()? "文件夹": QString("%1 文件").arg(suffix.toUpper()));
            model->setItem(count, 2, item);

            QString size = "";
            if(info.exists() && info.isFile()) {
                QString s_size = QString::number(info.size());
                quint64 num = 1;
                QString uu = "B";
                if(s_size.size() > 9) {
                    num = 1024*1024*1024;
                    uu = "GB";
                } else if(s_size.size() > 6) {
                    num = 1024*1024;
                    uu = "MB";
                } else if(s_size.size() > 3) {
                    num = 1024;
                    uu = "KB";
                }
                size = QString::number((double)info.size()/num, 'f', 2)+uu;
            }
            item = new QStandardItem(size);
            item->setToolTip(QString::number(info.size()));
            model->setItem(count, 3, item);
        }
    }
}

QMap<QString, QString> LocalFileManageApi::getFilesByDir(QString dir, QString currPath)
{
    dir = QDir::cleanPath(dir);
    currPath = QDir::cleanPath(currPath);

    QMap<QString, QString> res;

    QFileInfo fileInfo(dir);
    if(!fileInfo.exists() || !fileInfo.isDir()) {
        return res;
    }

    QFileInfoList fileInfoList = QDir(dir).entryInfoList();

    for(const QFileInfo& info : fileInfoList) {
        if(info.fileName() == "" || info.fileName() == "." || info.fileName() == "..") continue;

        QString path = info.filePath();
        QString p = dir;
        p.remove(currPath);
        if(info.isDir()) {
            res.insert(getFilesByDir(path, currPath));
        } else {
            res.insert(path, p);
        }
    }

    return res;
}
