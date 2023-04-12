#ifndef REMOTEFILEMANAGEMAINWIN_H
#define REMOTEFILEMANAGEMAINWIN_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "FileManageApi/RemoteFileManageApi.h"
#include <QInputDialog>
#include <QDir>
#include "FileOperation/FileOperation.h"
#include "SessionMsg/SessionMsg.h"
#include "AsycTask/AsycNetStatus.h"
#include "SessionMsg/SessionMsgDialog.h"
#include "DebugMsg/DebugMsgWidget.h"
#include "ScanCFCFile/ScanCFCFileWidget.h"
#include "ConnectServer/CountDown.h"
//#define REMOTEFILEMANAGEMAINWIN_DEBUG

#ifdef REMOTEFILEMANAGEMAINWIN_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#endif


extern SessionMsg giv_SessionMsg;

namespace Ui {
class RemoteFileManageMainWin;
}

class RemoteFileManageMainWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit RemoteFileManageMainWin(int fc, const QString &serverName, const QString &currPath, QWidget *parent = nullptr);
    ~RemoteFileManageMainWin();

    void setfc(int fc);
private:
    void createContextMenu();


    void slotFileOperationOpen();
public slots:
    /**
     * @brief 上传
     */
    void slotUpload(const QMap<QString, QString> &uploadFiles, const QString &clientPath);



    void slotAddDownloadFiles(const QStringList &list);
    void slotAddUploadFiles(const QStringList &list);

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
     * @brief 下载
     */
    void slotDownload();

    /**
     * @brief 选择的信息
     */
    void slotSelectedInfo(const QPoint &pos);

    /**
     * @brief 右键菜单
     */
    void slotContextRightClickMenu(const QPoint &pos);

    /**
     * @brief 获取网络状态
     */
    void slotRunNetStatus();

    void slotScanDialogOpen();

    void slotSessionMsgDialogOpen();

    void slotDebugWidgetOpen();

    void pushButtonReConn_clicked();

    void pushButton_fq_clicked();

signals:
    void sigDebugMsg(const QString &msg);
    void sigCreate();
    void sigQuit(const QString &name);
    void sigNoReConn();
    void sigReConnection(bool isExit_C);

private:
    Ui::RemoteFileManageMainWin *ui;

    int m_fc;
    QString m_ClientName;

    QMenu *m_RightClickMenu;
    QAction *m_ActionPaste;
    QAction *m_ActionCopy;
    QAction *m_ActionDelete;
    QAction *m_ActionCut;
    QAction *m_ActionRefresh;
    QAction *m_ActionOpen;
    QAction *m_ActionRename;

    QStringList m_ActionsStr;

    QStringList m_CopyList;
    bool m_Copy;

    QStandardItemModel *m_Model;
    RemoteFileManageApi m_RemoteFileManageApi;
    FileOperation *m_FileOperation;
    QLabel *m_LabelMsg;
    QLabel *m_DelayMsg;
    QLabel *m_UpSpeedMsg;
    QLabel *m_DownSpeedMsg;

    AsycNetStatus *m_AsycNetStatus;

    SessionMsgDialog *m_SessionMsgDialog;

    DebugMsgWidget *m_DebugMsgWidget;

    ScanCFCFileWidget *m_ScanCFCFileWidget;

    CountDown *m_CountDown;

    bool m_ISExit_C;
    bool m_SDReConn;

    bool flag = true;

};

#endif // REMOTEFILEMANAGEMAINWIN_H
