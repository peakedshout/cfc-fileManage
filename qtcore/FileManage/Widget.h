#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QSettings>
#include <QTabBar>
#include "FileManageWidget.h"
#include "SessionMsg/SessionMsg.h"
#include <QSystemTrayIcon>
#include<QMenu>

extern SessionMsg giv_SessionMsg;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:

    void closeEvent(QCloseEvent *e);

    void resizeEvent(QResizeEvent *e);

private:
    void tabWidgetTabClose(const QString &name);

    void creatSystemTray();

private slots:
    void slotTabWidgetTabCloseRequested(int index);
    void slotTabWidgetTabBarClicked(int index);

public slots:
    void slotCreateTabWidget(int fc, const QString &currPath, const QString &serverName, const QString &ipAndPort, const SessionInfo &info);

    void slotReconSucceed(const QString &clientName, const SessionInfo &pSessionInfo, int fc);
signals:
    void sigLogin();

    void sigQuit(const QString &ipAndPort);
    void sigCloseFc(int fc);
    void sigReConnection(const SessionInfo &pSessionInfo, bool isExit_C);
private:
    Ui::Widget *ui;

    int m_TabIndex;

    QMap<QString, FileManageWidget *> m_Map;
    QMap<QString, QString> m_LoginInfoMap;


    QMenu *m_TrayMenu;            //系统托盘右键菜单项
    QSystemTrayIcon *m_SystemTray; //系统托盘图标

    //右键菜单栏选项
    QAction *m_ActionShow;
    QAction *m_ActionHide;
    QAction *m_ActionQuit;
};

#endif // WIDGET_H
