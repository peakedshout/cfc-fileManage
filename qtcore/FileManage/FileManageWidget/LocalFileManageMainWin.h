#ifndef LOCALFILEMANAGEMAINWIN_H
#define LOCALFILEMANAGEMAINWIN_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "FileManageApi/LocalFileManageApi.h"
#include <QInputDialog>
#include <QFileSystemWatcher>
#include "DebugMsg/DebugMsgWidget.h"
#include "SessionMsg/SessionMsg.h"

extern SessionMsg giv_SessionMsg;

//#define LOCALFILEMANAGEMAINWIN_DEBUG

#ifdef LOCALFILEMANAGEMAINWIN_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#endif

namespace Ui {
class LocalFileManageMainWin;
}

class LocalFileManageMainWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit LocalFileManageMainWin(const QString &serverName, QWidget *parent = nullptr);
    ~LocalFileManageMainWin();

    void setWatchPath(const QString &path);
private:
    void createContextMenu();

private slots:
    void on_pushButton_Search_clicked();

    void slotActionTriggered(QAction *action);

    /**
     * @brief 上一级目录
     */
    void slotParentDir();
    /**
     * @brief 刷新
     */
    void slotRefresh();

    /**
     * @brief 打开文件或目录
     */
    void slotOpen();

    /**
     * @brief 复制文件或目录
     */
    void slotCopy();

    /**
     * @brief 粘贴
     */
    void slotPaste();

    /**
     * @brief 剪切
     */
    void slotCut();

    /**
     * @brief 删除
     */
    void slotDelete();

    /**
     * @brief 重命名
     */
    void slotRename();

    /**
     * @brief 上传
     */
    void slotUpload();

    /**
     * @brief 选择的信息
     */
    void slotSelectedInfo(const QPoint &pos);

    /**
     * @brief 右键菜单
     */
    void slotContextRightClickMenu(const QPoint &pos);

    void slotDebugWidgetOpen();

signals:
    void sigDebugMsg(const QString &msg);

    void sigUpload(const QMap<QString, QString> &uploadFiles, const QString &clientPath);

    void sigAddDownloadFiles(const QStringList &list);
    void sigAddUpFiles(const QStringList &list);

private:
    Ui::LocalFileManageMainWin *ui;

    QMenu *m_RightClickMenu;
    QAction *m_ActionPaste;
    QAction *m_ActionCopy;
    QAction *m_ActionDelete;
    QAction *m_ActionCut;
    QAction *m_ActionRefresh;
    QAction *m_ActionOpen;
    QAction *m_ActionRename;

    QString m_ClientName;

    QStringList m_ActionsStr;

    QStringList m_CopyList;
    bool m_Copy;

    QStandardItemModel *m_Model;
    LocalFileManageApi m_LocalFileManageApi;

    QFileSystemWatcher *m_FileSystemWatcher;

    QLabel *m_LabelMsg;

    DebugMsgWidget *m_DebugMsgWidget;
};

#endif // LOCALFILEMANAGEMAINWIN_H
