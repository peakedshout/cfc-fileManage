#include "Widget.h"
#include "ConnectServer/ConnectInfoDialog.h"
#include <QApplication>
#include "SessionMsg/SessionMsg.h"

QByteArray key("dfgahjkldsgh");
SessionMsg giv_SessionMsg(key);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    ConnectInfoDialog connectInfoDialog;
    connectInfoDialog.show();

    Widget w;

    QObject::connect(&connectInfoDialog, &ConnectInfoDialog::sigLoginMsg, &w, [&w, &connectInfoDialog](int fc,
                     const QString &currPath, const QString &serverName, const QString &ipAndPort) {
        w.slotCreateTabWidget(fc, currPath, serverName, ipAndPort);
        connectInfoDialog.hide();
        w.show();
    });
    QObject::connect(&w, &Widget::sigLogin, &connectInfoDialog, &ConnectInfoDialog::exec);
    QObject::connect(&w, &Widget::sigQuit, &connectInfoDialog, &ConnectInfoDialog::slotSessionQuit);

    w.hide();

    return a.exec();
}
