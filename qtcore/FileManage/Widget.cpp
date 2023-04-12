#include "Widget.h"
#include "ui_Widget.h"
#include "RewriteApi/cfcfile_api_win_amd64.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QString title = GetVersionInfo();
    setWindowTitle(title);

    m_TrayMenu = nullptr;            //系统托盘右键菜单项
    m_SystemTray = nullptr;   //系统托盘图标

    //右键菜单栏选项
    m_ActionShow = nullptr;
    m_ActionHide = nullptr;
    m_ActionQuit = nullptr;

    int w = giv_SessionMsg.width;
    int h = giv_SessionMsg.height;

    if(w < 1000 || h < 400) {
        this->resize(1200, 560);
    } else {
        this->resize(w, h);
    }

    // 取消Tab的关闭按钮
    ((QTabBar *)(ui->tabWidget->tabBar()))->setTabButton(ui->tabWidget->indexOf(ui->addTabW), QTabBar::RightSide, nullptr);


    connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &Widget::slotTabWidgetTabBarClicked);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &Widget::slotTabWidgetTabCloseRequested);

    creatSystemTray();
}

Widget::~Widget()
{
    giv_SessionMsg.recover();
    giv_SessionMsg.wirteSessionMsg();
    delete ui;
}

void Widget::closeEvent(QCloseEvent *e)
{
    QMessageBox msgBox;
    bool rejected = false;
    connect(&msgBox, &QMessageBox::rejected, this, [&rejected, e](){
        e->ignore();
        rejected = true;
    });
    msgBox.setWindowTitle("程序");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("是否退出程序？（Yes: 退出；No: 隐藏；Cancel: 取消）");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int btn = msgBox.exec();

    if(btn == QMessageBox::Yes) {
        e->accept();
    } else if(btn == QMessageBox::No) {
        e->ignore();
        this->hide();
    }

    if(rejected) return;

//    return QWidget::closeEvent(e);
}

void Widget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    giv_SessionMsg.height = this->height();
    giv_SessionMsg.width = this->width();
}

void Widget::tabWidgetTabClose(const QString &name)
{
    for(int index = 0; index < ui->tabWidget->count(); index++) {
        if(ui->tabWidget->tabText(index) == name) {

            FileManageWidget *w = m_Map.value(name);
            m_Map.remove(name);

            emit sigQuit(m_LoginInfoMap.value(name));
            m_LoginInfoMap.remove(name);

            ui->tabWidget->removeTab(index);

            if(w) w->deleteLater();

            if(index > 0) {
                ui->tabWidget->setCurrentIndex(index-1);
            }


            break;
        }
    }

}

void Widget::slotCreateTabWidget(int fc, const QString &currPath, const QString &clientName, const QString &ipAndPort, const SessionInfo &info)
{
    FileManageWidget *fileManageWidget = new FileManageWidget(fc, clientName, currPath, info,  this);

    connect(fileManageWidget, &FileManageWidget::sigCreate, this, &Widget::sigLogin);
    connect(fileManageWidget, &FileManageWidget::sigQuit, this, [=](const QString &name){
        emit sigQuit(name);
        tabWidgetTabClose(name);
    });
    connect(fileManageWidget, &FileManageWidget::sigReConnection, this, &Widget::sigReConnection);
    connect(fileManageWidget, &FileManageWidget::sigCloseFc, this, &Widget::sigCloseFc);


    if(m_Map.contains(clientName)) {
        m_Map.value(clientName)->deleteLater();
    }
    m_Map.insert(clientName, fileManageWidget);
    m_LoginInfoMap.insert(clientName, ipAndPort);

    QString name = clientName;
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        if(ui->tabWidget->tabText(i) == clientName) {
            QStringList res = ipAndPort.split("_");
            if(res.size() > 2) {
                name = res.at(2) + "_" + name;
                for(int i = 0; i < ui->tabWidget->count(); ++i) {
                    if(ui->tabWidget->tabText(i) == name) {
                        QStringList res = ipAndPort.split("_");
                        if(res.size() > 0) {
                            name = res.at(0) + "_" + name;
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    ui->tabWidget->insertTab(0, fileManageWidget, name);
    QMetaObject::invokeMethod(ui->tabWidget, "setCurrentIndex", Qt::QueuedConnection,
                              Q_ARG(int, 0));
}

void Widget::slotReconSucceed(const QString &clientName, const SessionInfo &pSessionInfo, int fc)
{
    if(m_Map.value(clientName))
        m_Map.value(clientName)->slotReconSucceed(clientName, pSessionInfo, fc);
}


void Widget::slotTabWidgetTabCloseRequested(int index)
{
    tabWidgetTabClose(ui->tabWidget->tabText(index));
}

void Widget::slotTabWidgetTabBarClicked(int index)
{
    if(index == ui->tabWidget->count() - 1) {
        emit sigLogin();
    } else {
        m_TabIndex = index;
    }

    QMetaObject::invokeMethod(ui->tabWidget, "setCurrentIndex", Qt::QueuedConnection, Q_ARG(int, m_TabIndex));
//    ui->tabWidget->setCurrentIndex(m_TabIndex);

}


void Widget::creatSystemTray()
{
    //创建菜单对象和托盘图标对象
    m_TrayMenu = new QMenu(this);
    m_SystemTray = new QSystemTrayIcon(this);

    //创建菜单项
    m_ActionShow = new QAction(tr("显示"), this);
    connect(m_ActionShow, &QAction::triggered, this, &Widget::show);

    m_ActionHide = new QAction(tr("隐藏"), this);
    connect(m_ActionHide, &QAction::triggered, this, &Widget::hide);

    m_ActionQuit = new QAction(tr("退出"), this);
    connect(m_ActionQuit, &QAction::triggered, this, &Widget::close);

    //添加菜单项
    m_TrayMenu->addAction(m_ActionShow);
    m_TrayMenu->addAction(m_ActionHide);
    m_TrayMenu->addSeparator();
    m_TrayMenu->addAction(m_ActionQuit);

    //为系统托盘设置菜单为m_pTrayMennu
    m_SystemTray->setContextMenu(m_TrayMenu);
    m_SystemTray->setIcon(QIcon(":/png/cfloge.png"));

    m_SystemTray->show();
}
