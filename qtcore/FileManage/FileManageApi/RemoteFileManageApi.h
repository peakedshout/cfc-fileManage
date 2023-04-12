#ifndef REMOTEFILEMANAGEAPI_H
#define REMOTEFILEMANAGEAPI_H

#include "VirtualFileManageApi.h"


//#define REMOTEFILEMANAGEAPI_DEBUG

#ifdef REMOTEFILEMANAGEAPI_DEBUG
    #include <QDebug>
    #define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
    #define debugMsg(msg)
#endif



class RemoteFileManageApi : public VirtualFileManageApi
{
public:
    RemoteFileManageApi();
    RemoteFileManageApi(int fc);
    ~RemoteFileManageApi();

    /**
     * @brief 设置fc值
     * @param fc
     */
    void setFC(int fc);

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
    void readerDir(const QString &path, QStandardItemModel *model, QString &curPath);
    void readerDir(const QByteArray &json, QStandardItemModel *model, int &fc, QString &curPath);

    /**
     * @brief 得到目录下的所有文件
     * @param Dir
     */
    QMap<QString, QString> getFilesByDir(QString dir, QString currPath);

private:
    int m_fc;

};

#endif // REMOTEFILEMANAGEAPI_H
