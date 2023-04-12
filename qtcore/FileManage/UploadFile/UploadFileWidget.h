#ifndef UPLOADFILEWIDGET_H
#define UPLOADFILEWIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QStringListModel>
#include <QThreadPool>
#include "AsycTask/AsycUploadFile.h"
#include "AsycTask/AsycGetProgress.h"
#include <QMessageBox>
#include <QListWidgetItem>
#include "SessionMsg/SessionMsg.h"

extern SessionMsg giv_SessionMsg;

//#define DEBUG

#ifdef DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

namespace Ui {
class UploadFileWidget;
}

class UploadFileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UploadFileWidget(int fc, const QString &clientName, QWidget *parent = nullptr);
    ~UploadFileWidget();

    void setfc(int fc);
private:
    bool createUploadTask(QString path, bool buff);
    void createItemAndProgressTask(QString path);

    void stopUpload(QString path);

public slots:
    void slotClientPath(const QString &path);
    void slotServerPath(const QString &path);
    void slotUploadFilesInfo(const QMap<QString, QString> &paths, bool clear = false);
    void on_pushButton_Upload_clicked();

    void slotAddUploadFiles(const QStringList &upFiles);
private slots:
    void on_pushButton_SeleteFile_clicked();

    void on_pushButton_Cancel_clicked();

    void slotCancel(QString path);

    void slotReset(QString from, QString to);

    void slotUploadState(QString path, bool running);

    void on_pushButton_DeleteItem_clicked();

    void on_pushButton_Close_clicked();

signals:
    void sigUploadFinished();

private:
    Ui::UploadFileWidget *ui;
    int m_fc;
    QString m_ClientName;

    QString m_ClientPath;
    QString m_ServerPath;

    QStringListModel *m_SeleteFilesModel;

    QThreadPool m_ThreadPool;

    QMap<QString, AsycUploadFile*> m_UploadTasks;
    QMap<QString, AsycGetProgress*> m_GetProgressTasks;
    QMap<QString, QListWidgetItem*> m_UploadFileItems;

    QMap<QString, QString> m_UploadFiles;

};

#endif // UPLOADFILEWIDGET_H
