#include "RemoteFileManageApi.h"
#include "RewriteApi/CFCFileApiParse.h"
#include "RewriteApi/GoStr.h"
#include <QDir>

RemoteFileManageApi::RemoteFileManageApi()
{
    m_fc = -1;
}

RemoteFileManageApi::RemoteFileManageApi(int fc)
{
    m_fc = fc;
}

RemoteFileManageApi::~RemoteFileManageApi()
{
}

void RemoteFileManageApi::setFC(int fc)
{
    m_fc = fc;
}

bool RemoteFileManageApi::open(const QString &path)
{
    if(m_fc <= 0) return false;
    GoStr p(path);
    QSharedPointer<char> msg(ReadRemoteFileContext(m_fc, p.getGoString(), 0, 100, 3000));
    QByteArray arr = QByteArray::fromBase64(msg.data());
    QString encodeMsg = QString::fromLatin1(arr.data());

    Q_UNUSED(encodeMsg);

    return false;
}

bool RemoteFileManageApi::copy(const QString &oldPath, const QString &newPath)
{
    if(m_fc <= 0) return false;
    GoStr oldP(oldPath);
    GoStr newP(newPath);
    QSharedPointer<char> msg(CopyRemoteFileContext(m_fc, oldP.getGoString(), newP.getGoString(), false));
    if(!QString(msg.data()).isEmpty()) {
        int btn = QMessageBox::information(nullptr, "文件存在", "文件已存在，是否覆盖", QMessageBox::Yes | QMessageBox::No , QMessageBox::Yes);

        if(btn == QMessageBox::No) return false;

        QSharedPointer<char> msg1(CopyRemoteFileContext(m_fc, oldP.getGoString(), newP.getGoString(), true));
        debugMsg(msg1.data());
        return QString(msg1.data()).isEmpty();
    }

    debugMsg(msg.data());
    return true;
}

bool RemoteFileManageApi::remove(const QString &path)
{
    if(m_fc <= 0) return false;

    GoStr p(path);
    QSharedPointer<char> msg(RemoveRemoteFileContext(m_fc, p.getGoString()));

    debugMsg(msg.data());

    return QString(msg.data()).isEmpty();
}

bool RemoteFileManageApi::move(const QString &oldPath, const QString &newPath)
{
    if(m_fc <= 0 && oldPath == newPath) return false;
    GoStr oldP(oldPath);
    GoStr newP(newPath);
    QSharedPointer<char> msg(MoveRemoteFileContext(m_fc, oldP.getGoString(), newP.getGoString(), false));
    if(!QString(msg.data()).isEmpty()) {
        int btn = QMessageBox::information(nullptr, "文件存在", "文件已存在，是否覆盖", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(btn == QMessageBox::No) return false;

        QSharedPointer<char> msg1(MoveRemoteFileContext(m_fc, oldP.getGoString(), newP.getGoString(), true));
        debugMsg(msg1.data());
        return QString(msg1.data()).isEmpty();
    }

    debugMsg(msg.data());
    return true;
}

void RemoteFileManageApi::readerDir(const QString &path, QStandardItemModel *model, QString &curPath)
{
    if(!model || m_fc <= 0) return;

    GoStr p(path);
    QSharedPointer<char> msg(OpenRemoteFileContext(m_fc, p.getGoString()));

    debugMsg(msg.data());

    QByteArray data(msg.data());
    readerDir(data, model, m_fc, curPath);
}

void RemoteFileManageApi::readerDir(const QByteArray &json, QStandardItemModel *model, int &fc, QString &curPath)
{
    FileContext fileContext;

    bool ok = CFCFileApiParse::fileContext(json, fileContext);
    if(!ok) {
        QMessageBox::information(nullptr, "情况", fileContext.error);
        return;
    }

    fc = fileContext.id;
    if(fileContext.current.isEmpty() || fileContext.current == ".") {
        curPath = "/";
    } else {
        curPath = fileContext.current;
    }

    if(!model) return;
    model->setRowCount(0);
    QStandardItem *item = nullptr;
    for(int i = 0; i < fileContext.fileList.size(); i++) {
        const FileContext::File &info = fileContext.fileList.at(i);
        if(info.name == "." || info.name == "..") continue;

        QFileInfo fileInfo(info.name);
        QString suffix = fileInfo.suffix();

        QString fileIconStr = QString(":/png/%1.png").arg(suffix);
        fileInfo.setFile(fileIconStr);
        if(!fileInfo.exists()) {
            fileIconStr = ":/png/what.png";
        }

        item = new QStandardItem(QIcon(info.isDir? ":/png/dir.png" : fileIconStr), info.name);
    //        item->setCheckable(true);
        model->setItem(i, 0, item);

        item = new QStandardItem(info.modTime);
        model->setItem(i, 1, item);

        item = new QStandardItem(info.isDir? "文件夹": QString("%1 文件").arg(suffix.toUpper()));
        model->setItem(i, 2, item);

        item = new QStandardItem(info.size);
        model->setItem(i, 3, item);
    }
}

QMap<QString, QString> RemoteFileManageApi::getFilesByDir(QString dir, QString currPath)
{
    QMap<QString, QString> res;
    if(m_fc <= 0) return res;

    GoStr p(dir);
    QSharedPointer<char> msg(OpenRemoteFileContext(m_fc, p.getGoString()));

    FileContext fileContext;

    QByteArray json(msg.data());
    bool ok = CFCFileApiParse::fileContext(json, fileContext);
    if(!ok) {
        return res;
    }

    if(dir == "/") dir = "";
    if(currPath == "/") currPath = "";

    for(int i = 0; i < fileContext.fileList.size(); i++) {
        const FileContext::File &info = fileContext.fileList.at(i);
        if(info.name == "." || info.name == "..") continue;

        QString path = QDir::cleanPath(dir + QDir::separator() + info.name);
        QString p = dir;
        p.remove(currPath);
        if(info.isDir) {
            res.insert(getFilesByDir(path, p));
        } else {
            res.insert(path, p);
        }
    }

    return res;
}

