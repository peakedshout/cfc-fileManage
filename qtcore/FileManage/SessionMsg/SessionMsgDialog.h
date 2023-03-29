#ifndef SESSIONMSGDIALOG_H
#define SESSIONMSGDIALOG_H

#include <QDialog>
#include "SessionMsg.h"
#include <QtCharts>
using namespace QtCharts;

namespace Ui {
class SessionMsgDialog;
}

extern SessionMsg giv_SessionMsg;

class SessionMsgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionMsgDialog(const QString &serverName, QWidget *parent = nullptr);
    ~SessionMsgDialog();

    void setSessionMsg(const QString &serverName);

private:
    void createpieSewies();

public slots:
    void slotMemoryCapacity(quint64 value, quint64 min, quint64 max, const QString &valueText, const QString &maxText);

private slots:
    void on_pushButton_Close_clicked();

    void on_pushButton_Quit_clicked();

    void on_pushButton_Create_clicked();

    void on_spinBox_SessionNum_valueChanged(int arg1);

    void on_spinBox_UploadNum_valueChanged(int arg1);

    void on_spinBox_DownloadNum_valueChanged(int arg1);

signals:
    void sigQuit(const QString &name);
    void sigCreate();

private:
    Ui::SessionMsgDialog *ui;

    QString m_ServerName;
    QPieSlice *m_PieSliceSum;
    QPieSlice *m_PieSlice;

};

#endif // SESSIONMSGDIALOG_H
