#ifndef DOWNLOADFILEWIDGET_H
#define DOWNLOADFILEWIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QStringListModel>
#include <QThreadPool>
#include "AsycTask/AsycDownloadFile.h"
#include "AsycTask/AsycGetProgress.h"
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QUrl>
#include "SessionMsg/SessionMsg.h"

extern SessionMsg giv_SessionMsg;

//#define DOWNLOADFILEWIDGET_DEBUG

#ifdef DOWNLOADFILEWIDGET_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

namespace Ui {
class DownloadFileWidget;
}

class DownloadFileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadFileWidget(int fc, const QString &serverName, QWidget *parent = nullptr);
    ~DownloadFileWidget();

    void setfc(int fc);
private:
    bool createDownloadTask(QString path, bool buff);
    void createItemAndProgressTask(QString path);

    void stopDownload(QString path);

public slots:
    void slotClientPath(const QString &path);
    void slotServerPath(const QString &path);
    void slotDownloadFilesInfo(const QMap<QString, QString> &paths);

    void slotAddDownloadFiles(const QStringList &downloadFiles);

private slots:
    void on_pushButton_Cancel_clicked();

    void on_pushButton_Download_clicked();

    void slotCancel(QString path);
    void slotReset(QString from, QString to);

    void slotDownloadState(QString path, bool running);

    void on_pushButton_DeleteItem_clicked();

    void on_pushButton_OpenDir_clicked();

    void on_pushButton_Close_clicked();

private:
    Ui::DownloadFileWidget *ui;
    int m_fc;
    QString m_ClientName;

    QString m_ClientPath;
    QString m_ServerPath;

    QStringListModel *m_SeleteFilesModel;

    QThreadPool m_ThreadPool;

    QMap<QString, AsycDownloadFile*> m_DownloadTasks;
    QMap<QString, AsycGetProgress*> m_GetProgressTasks;
    QMap<QString, QListWidgetItem*> m_DownloadFileItems;

    QMap<QString, QString> m_DownloadFiles;

};

#endif // DOWNLOADFILEWIDGET_H
