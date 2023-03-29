#ifndef VIRTUALFILEMANAGEAPI_H
#define VIRTUALFILEMANAGEAPI_H

#include <QString>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QFileInfo>

class VirtualFileManageApi
{
public:
    virtual ~VirtualFileManageApi() {};

    /**
     * @brief 打开文件
     * @param path
     * @return
     */
    virtual bool open(const QString &path) {Q_UNUSED(path); return false;}

    /**
     * @brief 复制文件
     * @param oldPath
     * @param newPath
     * @return
     */
    virtual bool copy(const QString &oldPath, const QString &newPath) {Q_UNUSED(oldPath); Q_UNUSED(newPath); return false;}

    /**
     * @brief 删除文件
     * @param path
     * @return
     */
    virtual bool remove(const QString &path) {Q_UNUSED(path); return false;}

    /**
     * @brief 移动文件
     * @param oldPath
     * @param newPath
     * @return
     */
    virtual bool move(const QString &oldPath, const QString &newPath) {Q_UNUSED(oldPath); Q_UNUSED(newPath); return false;}

    /**
     * @brief 渲染目录
     * @param path
     */
    void readerDir(const QString &path, QStandardItemModel *model) {Q_UNUSED(path); Q_UNUSED(model);}
    void readerDir(const QByteArray &json, QStandardItemModel *model) {Q_UNUSED(json); Q_UNUSED(model);}

    /**
     * @brief 得到目录下的所有文件
     * @param Dir
     */
    QMap<QString, QString> getFilesByDir(QString dir, QString currPath) {Q_UNUSED(dir); Q_UNUSED(currPath);QMap<QString, QString> res; return res;}
};

#endif // VIRTUALFILEMANAGEAPI_H
