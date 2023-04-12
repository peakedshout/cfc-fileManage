#ifndef CONNECTINFODIALOG_H
#define CONNECTINFODIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include "SessionMsg/SessionMsg.h"
#include "CManage.h"

extern SessionMsg giv_SessionMsg;

//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

namespace Ui {
class ConnectInfoDialog;
}

class ConnectInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectInfoDialog(QWidget *parent = nullptr);
    ~ConnectInfoDialog();

    void slotSessionQuit(const QString &ipAndPort);
    void slotReConnection(const SessionInfo &pSessionInfo, bool isExit_C);

    void slotClose(int fc);
private:
    void sessionMsg();

private slots:
    void on_pushButton_selectDir_clicked();

    void on_pushButton_connect_clicked();

    void on_pushButton_selectDir_2_clicked();

    void on_pushButton_selectDir_3_clicked();

    void on_pushButton_Del_clicked();

    void on_toolButton_2_toggled(bool checked);

    void on_toolButton_toggled(bool checked);

signals:
    void sigLoginMsg(int fc, const QString &currPath, const QString &clientName, const QString &ipAndPort, const SessionInfo &info);

    void sigReconSucceed(const QString &clientName, const SessionInfo &pSessionInfo, int fc);
private:
    Ui::ConnectInfoDialog *ui;
    QString m_ClientName;

    QList<QString> m_LoginList;// QString(ip+port)
    QMap<QString, int> m_RegisterMap;// QString(ip+port), c

    int num = 0;

    CManage m_CManage;
};

#endif // CONNECTINFODIALOG_H
