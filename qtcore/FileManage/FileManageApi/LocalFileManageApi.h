#ifndef LOCALFILEMANAGEAPI_H
#define LOCALFILEMANAGEAPI_H

#include "VirtualFileManageApi.h"
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QDesktopServices>
#include <QDateTime>


//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class LocalFileManageApi : public VirtualFileManageApi
{
public:
    LocalFileManageApi();
    ~LocalFileManageApi();

    /**
     * @brief 打开文件
     * @param path
     * @return
     */
    bool open(const QString &path);

    /**
     * @brief 复制文件
     * @param oldPath
     * @param newPath
     * @return
     */
    bool copy(const QString &oldPath, const QString &newPath);
    bool copyFile(const QString &oldPath, const QString &newPath, QMessageBox::StandardButton &state);
    bool copyDir(const QString &oldPath, const QString &newPath, QMessageBox::StandardButton &state);

    /**
     * @brief 删除文件
     * @param path
     * @return
     */
    bool remove(const QString &path);

    /**
     * @brief 移动文件
     * @param oldPath
     * @param newPath
     * @return
     */
    bool move(const QString &oldPath, const QString &newPath);

    /**
     * @brief 渲染目录
     * @param path
     * @param model
     */
    void readerDir(const QString &path, QStandardItemModel *model);

    /**
     * @brief 得到目录下的所有文件
     * @param Dir
     */
    QMap<QString, QString> getFilesByDir(QString dir, QString currPath);
};

#endif // LOCALFILEMANAGEAPI_H
