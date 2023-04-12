#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QDialog>
#include <QFileDialog>
#include <QStringListModel>
#include <QThreadPool>
#include "AsycTask/AsycDownloadFile.h"
#include "AsycTask/AsycUploadFile.h"
#include "AsycTask/AsycGetProgress.h"

#include <QMessageBox>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QUrl>
#include "SessionMsg/SessionMsg.h"

extern SessionMsg giv_SessionMsg;

namespace Ui {
class FileOperation;
}

class FileOperation : public QDialog
{
    Q_OBJECT

public:
    explicit FileOperation(int fc, const QString &clientName, QWidget *parent = nullptr);
    ~FileOperation();

     void setfc(int fc);

private:
    bool createUploadTask(QString path, bool buff);
    void createUpItemAndProgressTask(QString path);
    void stopUpload(QString path);


    void stopDownload(QString path);
    bool createDownloadTask(QString path, bool buff);
    void createDownItemAndProgressTask(QString path);



public slots:
    void slotClientPath(const QString &path);
    void slotServerPath(const QString &path);
    void slotUploadFilesInfo(const QMap<QString, QString> &paths, bool clear = false);
    void slotAddUploadFiles(const QStringList &upFiles);
    void slotUploadClicked();


    void slotDownloadFilesInfo(const QMap<QString, QString> &paths);
    void slotAddDownloadFiles(const QStringList &downloadFiles);
    void slotDownloadClicked();
    void slotDownCancel(QString path);
    void slotDownReset(QString from, QString to);
    void slotDownloadState(QString path, bool running);

private slots:
    void slotUpCancel(QString path);
    void slotUpReset(QString from, QString to);
    void slotUploadState(QString path, bool running);

private slots:
    void on_pushButton_DownloadDir_clicked();
    void on_pushButton_Operation_clicked();
    void on_pushButton_DeleteItem_clicked();
    void on_pushButton_Cancel_clicked();
    void on_pushButton_Close_clicked();
    void on_pushButton_ScanFile_clicked();
    void on_pushButton_SeleteFile_clicked();

signals:
    void sigScanFiles();
    void sigUploadFinished();
private:
    Ui::FileOperation *ui;

    int m_fc;
    QString m_ClientName;

    QString m_ClientPath;
    QString m_ServerPath;

    QStringListModel *m_SeleteFilesModel;

    QThreadPool m_ThreadPool;

    QMap<QString, AsycUploadFile*> m_UploadTasks;
    QMap<QString, AsycGetProgress*> m_UploadGetProgressTasks;
    QMap<QString, QListWidgetItem*> m_UploadFileItems;

    QMap<QString, QString> m_UploadFiles;



    QMap<QString, AsycDownloadFile*> m_DownloadTasks;
    QMap<QString, AsycGetProgress*> m_DownGetProgressTasks;
    QMap<QString, QListWidgetItem*> m_DownloadFileItems;

    QMap<QString, QString> m_DownloadFiles;




};

#endif // FILEOPERATION_H
