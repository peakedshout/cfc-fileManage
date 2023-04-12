#ifndef FILEMANAGEWIDGET_H
#define FILEMANAGEWIDGET_H

#include <QWidget>
#include "FileManageWidget/LocalFileManageMainWin.h"
#include "FileManageWidget/RemoteFileManageMainWin.h"
#include <QHBoxLayout>
#include "ConnectServer/CManage.h"
class FileManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileManageWidget(int fc, const QString &serverName, const QString &currPath, const SessionInfo &info, QWidget *parent = nullptr);
    ~FileManageWidget();


    void slotReconSucceed(const QString &clientName, const SessionInfo &pSessionInfo, int fc);
signals:
    void sigCreate();
    void sigQuit(const QString &name);
    void sigReConnection(const SessionInfo &pSessionInfo, bool isExit_C);

    void sigCloseFc(int fc);
private:
    LocalFileManageMainWin *m_LocalWin;
    RemoteFileManageMainWin *m_RemoteWin;
    SessionInfo m_pSessionInfo;
    int m_fc;
};

#endif // FILEMANAGEWIDGET_H
