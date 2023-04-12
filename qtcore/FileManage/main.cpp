#include "Widget.h"
#include "ConnectServer/ConnectInfoDialog.h"
#include <QApplication>
#include "SessionMsg/SessionMsg.h"

QByteArray key("dfgahjkldsgh");
SessionMsg giv_SessionMsg(key);

int main(int argc, char *argv[])
{
    //环境变量设置
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    //确保只运行一次
    QSystemSemaphore sema("FileManage", 1, QSystemSemaphore::Open);
    sema.acquire();//在临界区操作共享内存  SharedMemory

    QSharedMemory mem("FileManageObject"); //全局对象名
    bool bCreate = mem.create(1);

    if(!bCreate) {
        //创建失败，说明已经有一个程序在运行了。
        sema.release();//如果是 Unix 系统，会自动释放。
        return 0;
    }

    sema.release();//临界区

    ConnectInfoDialog connectInfoDialog;
    connectInfoDialog.show();

    Widget w;

    QObject::connect(&connectInfoDialog, &ConnectInfoDialog::sigLoginMsg, &w, [&w, &connectInfoDialog](int fc,
                     const QString &currPath, const QString &clientName, const QString &ipAndPort, const SessionInfo &info) {
        w.slotCreateTabWidget(fc, currPath, clientName, ipAndPort, info);
        connectInfoDialog.hide();
        w.show();
    });

    QObject::connect(&connectInfoDialog, &ConnectInfoDialog::sigReconSucceed, &w, &Widget::slotReconSucceed);
    QObject::connect(&w, &Widget::sigLogin, &connectInfoDialog, &ConnectInfoDialog::exec);
    QObject::connect(&w, &Widget::sigQuit, &connectInfoDialog, &ConnectInfoDialog::slotSessionQuit);
    QObject::connect(&w, &Widget::sigReConnection, &connectInfoDialog, &ConnectInfoDialog::slotReConnection);
    QObject::connect(&w, &Widget::sigCloseFc, &connectInfoDialog, &ConnectInfoDialog::slotClose);

    w.hide();

    return a.exec();
}
