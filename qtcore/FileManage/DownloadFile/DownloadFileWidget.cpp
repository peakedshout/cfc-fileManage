#include "DownloadFileWidget.h"
#include "ui_DownloadFileWidget.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/ScanCFCFile.h"
#include "DownloadFileItem.h"

DownloadFileWidget::DownloadFileWidget(int fc, const QString &serverName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadFileWidget)
{
    ui->setupUi(this);

    m_fc = fc;
    m_ClientName = serverName;

    this->setWindowTitle("下载");

    m_SeleteFilesModel = new QStringListModel(ui->listView_FileList);
    ui->listView_FileList->setEditTriggers(QListView::NoEditTriggers);			//不能编辑
    ui->listView_FileList->setSelectionBehavior(QListView::SelectRows);		//一次选中整行
    ui->listView_FileList->setSelectionMode(QListView::ExtendedSelection);       //SingleSelection


    ui->listView_FileList->setModel(m_SeleteFilesModel);

    qRegisterMetaType<ProgressContext>("ProgressContext");



}

DownloadFileWidget::~DownloadFileWidget()
{
    if(m_fc > 0) CloseFileContext(m_fc);
    delete ui;
}

void DownloadFileWidget::setfc(int fc)
{
    m_fc = fc;
}

bool DownloadFileWidget::createDownloadTask(QString path, bool buff)
{
    path.remove(".CFCDownload_info");

//    QString p = path.mid(path.lastIndexOf('/')+1);

    // 删除下载
    if(m_DownloadTasks.contains(path)) {
        int btn = QMessageBox::information(this, "下载情况", "下载任务已存在，是否覆盖", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(btn == QMessageBox::No) return false;

        AsycDownloadFile *task = m_DownloadTasks.value(path);
        task->stopDownloadTask();
        task->deleteLater();
    }


    AsycDownloadFile *asycUploadFile = new AsycDownloadFile(m_fc, path, buff);
    m_DownloadTasks.insert(path, asycUploadFile);

    m_ThreadPool.start(asycUploadFile);

    return true;
}

void DownloadFileWidget::createItemAndProgressTask(QString path)
{
    path.remove(".CFCDownload_info");

    // 删除获取进度
    if(m_GetProgressTasks.contains(path)) {
        AsycGetProgress *task = m_GetProgressTasks.value(path);
        task->stop();
        task->deleteLater();
    }

//    QString p = path.mid(path.lastIndexOf('/')+1);
    AsycGetProgress *getProgress = new AsycGetProgress(m_fc, path, AsycGetProgress::Download);
    m_GetProgressTasks.insert(path, getProgress);

    //
    if(m_DownloadFileItems.contains(path)) {
        QListWidgetItem *item = m_DownloadFileItems.value(path);
        if(item) delete item;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(0, 100));
    DownloadFileItem *downloadFileItem = new DownloadFileItem(ui->listWidget);
    downloadFileItem->slotFileTo(path);

    connect(downloadFileItem, &DownloadFileItem::sigCancel, this, &DownloadFileWidget::slotCancel);
    connect(downloadFileItem, &DownloadFileItem::sigDownloadState, this, &DownloadFileWidget::slotDownloadState);
    connect(downloadFileItem, &DownloadFileItem::sigReset, this, &DownloadFileWidget::slotReset);
    connect(getProgress, &AsycGetProgress::sigDownloadFileProgress, downloadFileItem, &DownloadFileItem::slotDownloadProgress);


    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, downloadFileItem);
    m_DownloadFileItems.insert(path, item);

    // 开始任务
    m_ThreadPool.start(getProgress);
}

void DownloadFileWidget::stopDownload(QString path)
{
    path.remove(".CFCDownload_info");

    if(m_DownloadTasks.contains(path)) {
        auto item = m_DownloadTasks.value(path);
        if(item) {
            item->stopDownloadTask();
        }
    }

    if(m_GetProgressTasks.contains(path)) {
        auto item = m_GetProgressTasks.value(path);
        if(item) {
            item->stop();
        }
    }

}

void DownloadFileWidget::slotServerPath(const QString &path)
{
    m_ServerPath = path;
}

void DownloadFileWidget::slotClientPath(const QString &path)
{
    m_ClientPath = path;
}

void DownloadFileWidget::slotDownloadFilesInfo(const QMap<QString, QString> &paths)
{
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Download->setEnabled(true);

    m_DownloadFiles.insert(paths);
    QStringList &&list = m_DownloadFiles.keys();

    m_SeleteFilesModel->setStringList(list);
}

void DownloadFileWidget::slotAddDownloadFiles(const QStringList &downloadFiles)
{
    this->show();
    this->activateWindow();
    foreach (const QString &var, downloadFiles) {
        if(createDownloadTask(var, true)) {
            createItemAndProgressTask(var);
        }
    }
}

void DownloadFileWidget::on_pushButton_Cancel_clicked()
{
    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_DownloadFiles.clear();
    ui->pushButton_Download->setEnabled(false);
}

void DownloadFileWidget::on_pushButton_Download_clicked()
{
    if(!m_SeleteFilesModel) return;
    m_ThreadPool.setMaxThreadCount(giv_SessionMsg.sessions.value(m_ClientName).downloadNum*2);

//    ui->pushButton_Download->setEnabled(false);
    QStringList &&downloadList = m_DownloadFiles.keys();
    QStringList &&dirs = m_DownloadFiles.values();

    for(int i = 0; i < downloadList.size(); i++) {
        // 创建目录
        QDir dir(m_ClientPath);
        QString createPath = dir.path()+dirs.at(i);
        dir.mkpath(createPath);

        const QStringList &&scanList = ScanCFCFile::scanCFCFile(m_fc, createPath, ScanCFCFile::Download);

        QString var = downloadList.at(i);
        QString p = var.mid(var.lastIndexOf('/')+1);

        QString download = QDir::cleanPath(createPath + QDir::separator() + p);
        if(download.lastIndexOf(".CFCDownload_info") == -1) {
            download = download + ".CFCDownload_info";
        }

        bool &&buff = scanList.contains(download);

        if(!buff) {
            GoStr path(var);

            GoStr serverPath(createPath + "/" + p);
            QSharedPointer<char> msg(DownRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), false, 0));
            if(!QString(msg.data()).contains("\"ErrMsg\":\"\"")) {
                int btn = QMessageBox::information(this, "下载情况", msg.data(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if(btn == QMessageBox::No) return;
                QFile::remove(createPath + "/" + p);
                QSharedPointer<char> msg1(DownRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
                if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
                    QMessageBox::critical(this, "下载情况", QString("下载错误(%1)").arg(msg1.data()));
                    return;
                }
            }
            debugMsg(msg.data());
        }

        if(createDownloadTask(QDir::cleanPath(createPath + QDir::separator() + p), buff)) {
            createItemAndProgressTask(QDir::cleanPath(createPath + QDir::separator() + p));
        }
    }

    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_DownloadFiles.clear();
//    on_pushButton_Cancel_clicked();
}

void DownloadFileWidget::slotCancel(QString path)
{
    path.remove(".CFCDownload_info");

    if(m_DownloadTasks.contains(path)) {
        auto item = m_DownloadTasks.value(path);
        if(item) {
            item->stopDownloadTask();
            item->deleteLater();
        }
        m_DownloadTasks.remove(path);
    }

    if(m_GetProgressTasks.contains(path)) {
        auto item = m_GetProgressTasks.value(path);
        if(item) {
            item->stop();
            item->deleteLater();
        }
        m_GetProgressTasks.remove(path);
    }

    if(m_DownloadFileItems.contains(path)) {
        auto item = m_DownloadFileItems.value(path);
        if(item) {
            ui->listWidget->removeItemWidget(item);
            delete item;
        }
        m_DownloadFileItems.remove(path);
    }
}

void DownloadFileWidget::slotReset(QString from, QString to)
{
    GoStr path(from);
    GoStr serverPath(to);
    QSharedPointer<char> msg1(DownRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
    if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
        QMessageBox::critical(this, "下载重置情况", QString("下载重置错误(%1)").arg(msg1.data()));
        return;
    }
}

void DownloadFileWidget::slotDownloadState(QString path, bool running)
{
    path.remove(".CFCDownload_info");

    if(m_DownloadTasks.contains(path)) {
        auto item = m_DownloadTasks.value(path);
        if(item) {
            if(running) {
                m_ThreadPool.start(item);
            } else {
                item->stopDownloadTask();
            }
        }
    }

    if(m_GetProgressTasks.contains(path)) {
        auto item = m_GetProgressTasks.value(path);
        if(item) {
            if(running) {
                m_ThreadPool.start(item);
            } else {
                item->stop();
            }
        }
    }
}

void DownloadFileWidget::on_pushButton_DeleteItem_clicked()
{
    if(!m_SeleteFilesModel) return;
    QModelIndexList selIndexs = ui->listView_FileList->selectionModel()->selectedRows();
    foreach (const QModelIndex& index, selIndexs) {
        QString key = m_SeleteFilesModel->index(index.row()).data().toString();
        m_SeleteFilesModel->removeRow(index.row());
        m_DownloadFiles.remove(key);
    }
}


void DownloadFileWidget::on_pushButton_OpenDir_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_ClientPath));
}


void DownloadFileWidget::on_pushButton_Close_clicked()
{
    this->close();
}

