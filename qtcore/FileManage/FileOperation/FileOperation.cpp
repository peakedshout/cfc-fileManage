#include "FileOperation.h"
#include "ui_FileOperation.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/ScanCFCFile.h"
#include "UploadFile/UploadFileItem.h"
#include "DownloadFile/DownloadFileItem.h"

FileOperation::FileOperation(int fc, const QString &clientName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileOperation)
{
    ui->setupUi(this);


    m_fc = fc;
    m_ClientName = clientName;

    this->setWindowTitle("上传/下载");

    m_SeleteFilesModel = new QStringListModel(ui->listView_FileList);
    ui->listView_FileList->setEditTriggers(QListView::NoEditTriggers);			//不能编辑
    ui->listView_FileList->setSelectionBehavior(QListView::SelectRows);		//一次选中整行
    ui->listView_FileList->setSelectionMode(QListView::ExtendedSelection);       //SingleSelection

    ui->listView_FileList->setModel(m_SeleteFilesModel);

    qRegisterMetaType<ProgressContext>("ProgressContext");


}

FileOperation::~FileOperation()
{
    delete ui;
}

void FileOperation::setfc(int fc)
{
    m_fc = fc;
}

void FileOperation::on_pushButton_DownloadDir_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_ClientPath));
}

void FileOperation::on_pushButton_Operation_clicked()
{
    slotUploadClicked();
    slotDownloadClicked();

    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());
}

void FileOperation::on_pushButton_DeleteItem_clicked()
{
    if(!m_SeleteFilesModel) return;
    QModelIndexList selIndexs = ui->listView_FileList->selectionModel()->selectedRows();
    foreach (const QModelIndex& index, selIndexs) {
        QString key = m_SeleteFilesModel->index(index.row()).data().toString();
        m_SeleteFilesModel->removeRow(index.row());
        if(key.startsWith("[U]")) {
            m_UploadFiles.remove(key);
        } else if(key.startsWith("[D]")){
            m_DownloadFiles.remove(key);
        }
    }
}

void FileOperation::on_pushButton_Cancel_clicked()
{
    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_UploadFiles.clear();
    m_DownloadFiles.clear();
    ui->pushButton_Operation->setEnabled(false);
}

void FileOperation::on_pushButton_Close_clicked()
{
    this->close();
}

void FileOperation::on_pushButton_ScanFile_clicked()
{
    emit sigScanFiles();
}

void FileOperation::on_pushButton_SeleteFile_clicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("上传文件"), m_ClientPath);
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Operation->setEnabled(true);


    for(const auto &path : paths) {
        QFileInfo fileInfo(path);
        QString p = fileInfo.path();
        p.remove(m_ClientPath);
        m_UploadFiles.insert(path, p);
    }

    QStringList &&list = m_UploadFiles.keys();
    m_SeleteFilesModel->setStringList(list);
}

// 上传

bool FileOperation::createUploadTask(QString path, bool buff)
{
    path.remove(".CFCUpload_info");

    // 删除上传
    if(m_UploadTasks.contains(path)) {
        int btn = QMessageBox::information(this, "上传情况", "上传任务已存在，是否覆盖", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(btn == QMessageBox::No) return false;

        AsycUploadFile *task = m_UploadTasks.value(path);
        task->stopUploadTask();
        task->deleteLater();
    }


    AsycUploadFile *asycUploadFile = new AsycUploadFile(m_fc, path, buff);

    connect(asycUploadFile, &AsycUploadFile::sigDebug, this, [this](QString text, QString path){
            slotUpCancel(path);
            QMessageBox::critical(this, "错误", text);

    });
    m_UploadTasks.insert(path, asycUploadFile);
    m_ThreadPool.start(asycUploadFile);


    return true;
}

void FileOperation::createUpItemAndProgressTask(QString path)
{
    path.remove(".CFCUpload_info");

    // 删除获取进度
    if(m_UploadGetProgressTasks.contains(path)) {
        AsycGetProgress *task = m_UploadGetProgressTasks.value(path);
        task->stop();
        task->deleteLater();
    }

    AsycGetProgress *getProgress = new AsycGetProgress(m_fc, path, AsycGetProgress::Upload);
    m_UploadGetProgressTasks.insert(path, getProgress);

    //
    if(m_UploadFileItems.contains(path)) {
        QListWidgetItem *item = m_UploadFileItems.value(path);
        if(item) delete item;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(0, 100));
    UploadFileItem *uploadFileItem = new UploadFileItem(ui->listWidget);
    connect(uploadFileItem, &UploadFileItem::sigCancel, this, &FileOperation::slotUpCancel);
    connect(uploadFileItem, &UploadFileItem::sigReset, this, &FileOperation::slotUpReset);
    connect(uploadFileItem, &UploadFileItem::sigUploadState, this, &FileOperation::slotUploadState);

    connect(getProgress, &AsycGetProgress::sigDownloadFileProgress, uploadFileItem, &UploadFileItem::slotDownloadProgress);
    connect(getProgress, &AsycGetProgress::sigUploadFinished, this, &FileOperation::sigUploadFinished);

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, uploadFileItem);
    m_UploadFileItems.insert(path, item);

    // 开始任务
    m_ThreadPool.start(getProgress);
}

void FileOperation::stopUpload(QString path)
{
    path.remove(".CFCUpload_info");

    if(m_UploadTasks.contains(path)) {
        auto item = m_UploadTasks.value(path);
        if(item) {
            item->stopUploadTask();
        }
    }

    if(m_UploadGetProgressTasks.contains(path)) {
        auto item = m_UploadGetProgressTasks.value(path);
        if(item) {
            item->stop();
        }
    }

}

void FileOperation::slotServerPath(const QString &path)
{
    m_ServerPath = path;
}

void FileOperation::slotUploadFilesInfo(const QMap<QString, QString> &paths, bool clear)
{
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Operation->setEnabled(true);

    if(clear) m_UploadFiles.clear();

    m_UploadFiles.insert(paths);
    QStringList &&list = m_UploadFiles.keys();

    QStringList l = m_SeleteFilesModel->stringList();
    foreach (const QString &var, list) {
        QString text("[U]" + var);
        if(!l.contains(text)) {
            l <<  text;
        }
    }
    m_SeleteFilesModel->setStringList(l);
}

void FileOperation::slotClientPath(const QString &path)
{
    m_ClientPath = path;
}

void FileOperation::slotAddUploadFiles(const QStringList &upFiles)
{
    this->show();
    this->activateWindow();
    foreach (const QString &var, upFiles) {
        if(createUploadTask(var, true)) {
            createUpItemAndProgressTask(var);
        }
    }
}

void FileOperation::slotUploadClicked()
{
    if(!m_SeleteFilesModel) return;
    m_ThreadPool.setMaxThreadCount(giv_SessionMsg.sessions.value(m_ClientName).uploadNum*2 + giv_SessionMsg.sessions.value(m_ClientName).downloadNum*2);

//    ui->pushButton_Upload->setEnabled(false);
    QStringList &&uploadList = m_UploadFiles.keys();
    QStringList &&dirs = m_UploadFiles.values();

    for(int i = 0; i < uploadList.size(); i++) {
        QString var = uploadList.at(i);
        QFileInfo fileInfo(var);

        const QStringList &&scanList = ScanCFCFile::scanCFCFile(m_fc, giv_SessionMsg.sessions.value(m_ClientName).scanPath, ScanCFCFile::Upload);

        QString p = var.mid(var.lastIndexOf('/')+1);

        QString upload = var;
        if(upload.lastIndexOf(".CFCUpload_info") == -1) {
            upload = upload + ".CFCUpload_info";
        }

        bool &&buff = scanList.contains(upload);

        if(!buff) {
            GoStr path(var);

            GoStr serverPath(m_ServerPath + dirs.at(i) + "/" + p);
            QSharedPointer<char> msg2(MkDirRemoteFileContext(m_fc, GoStr(m_ServerPath + dirs.at(i)).getGoString()));

            QSharedPointer<char> msg(UpRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), false, 0));

            if(!QString(msg.data()).contains("\"ErrMsg\":\"\"")) {
                int btn = QMessageBox::information(this, "上传情况", msg.data(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if(btn == QMessageBox::No) return;
                QSharedPointer<char> msg1(UpRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
                if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
                    QMessageBox::critical(this, "上传情况", QString("上传错误(%1)").arg(msg1.data()));
                    return;
                }
            }
            debugMsg(msg.data());
        }

        if(createUploadTask(var, buff)) {
            createUpItemAndProgressTask(var);
        }

    }

//    on_pushButton_Cancel_clicked();
//    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_UploadFiles.clear();
}

void FileOperation::slotUpCancel(QString path)
{
    path.remove(".CFCUpload_info");

    if(m_UploadTasks.contains(path)) {
        auto item = m_UploadTasks.value(path);
        if(item) {
            item->stopUploadTask();
            item->deleteLater();
        }
        m_UploadTasks.remove(path);
    }

    if(m_UploadGetProgressTasks.contains(path)) {
        auto item = m_UploadGetProgressTasks.value(path);
        if(item) {
            item->stop();
            item->deleteLater();
        }
        m_UploadGetProgressTasks.remove(path);
    }

    if(m_UploadFileItems.contains(path)) {
        auto item = m_UploadFileItems.value(path);
        if(item) {
            ui->listWidget->removeItemWidget(item);
            delete item;
        }
        m_UploadFileItems.remove(path);
    }
}

void FileOperation::slotUpReset(QString from, QString to)
{
    GoStr path(from);
    GoStr serverPath(to);
    QSharedPointer<char> msg1(UpRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
    if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
        QMessageBox::critical(this, "上传重置情况", QString("上传重置错误(%1)").arg(msg1.data()));
        return;
    }
}

void FileOperation::slotUploadState(QString path, bool running)
{
    path.remove(".CFCUpload_info");


    if(m_UploadTasks.contains(path)) {
        auto item = m_UploadTasks.value(path);
        if(item) {
            if(running) {
                m_ThreadPool.start(item);
            } else {
                item->stopUploadTask();
            }
        }
    }

    if(m_UploadGetProgressTasks.contains(path)) {
        auto item = m_UploadGetProgressTasks.value(path);
        if(item) {
            if(running) {
                m_ThreadPool.start(item);
            } else {
                item->stop();
            }
        }
    }

}



// 下载
bool FileOperation::createDownloadTask(QString path, bool buff)
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

    AsycDownloadFile *asycDownloadFile = new AsycDownloadFile(m_fc, path, buff);
    connect(asycDownloadFile, &AsycDownloadFile::sigDebug, this, [this](QString text, QString path){
            slotDownCancel(path);
            QMessageBox::critical(this, "错误", text);

    });
    m_DownloadTasks.insert(path, asycDownloadFile);

    m_ThreadPool.start(asycDownloadFile);

    return true;
}

void FileOperation::createDownItemAndProgressTask(QString path)
{
    path.remove(".CFCDownload_info");

    // 删除获取进度
    if(m_DownGetProgressTasks.contains(path)) {
        AsycGetProgress *task = m_DownGetProgressTasks.value(path);
        task->stop();
        task->deleteLater();
    }

//    QString p = path.mid(path.lastIndexOf('/')+1);
    AsycGetProgress *getProgress = new AsycGetProgress(m_fc, path, AsycGetProgress::Download);
    m_DownGetProgressTasks.insert(path, getProgress);

    //
    if(m_DownloadFileItems.contains(path)) {
        QListWidgetItem *item = m_DownloadFileItems.value(path);
        if(item) delete item;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(0, 100));
    DownloadFileItem *downloadFileItem = new DownloadFileItem(ui->listWidget);
    downloadFileItem->slotFileTo(path);

    connect(downloadFileItem, &DownloadFileItem::sigCancel, this, &FileOperation::slotDownCancel);
    connect(downloadFileItem, &DownloadFileItem::sigDownloadState, this, &FileOperation::slotDownloadState);
    connect(downloadFileItem, &DownloadFileItem::sigReset, this, &FileOperation::slotDownReset);
    connect(getProgress, &AsycGetProgress::sigDownloadFileProgress, downloadFileItem, &DownloadFileItem::slotDownloadProgress);


    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, downloadFileItem);
    m_DownloadFileItems.insert(path, item);

    // 开始任务
    m_ThreadPool.start(getProgress);
}

void FileOperation::stopDownload(QString path)
{
    path.remove(".CFCDownload_info");

    if(m_DownloadTasks.contains(path)) {
        auto item = m_DownloadTasks.value(path);
        if(item) {
            item->stopDownloadTask();
        }
    }

    if(m_DownGetProgressTasks.contains(path)) {
        auto item = m_DownGetProgressTasks.value(path);
        if(item) {
            item->stop();
        }
    }

}

void FileOperation::slotDownloadFilesInfo(const QMap<QString, QString> &paths)
{
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Operation->setEnabled(true);

    m_DownloadFiles.insert(paths);
    QStringList &&list = m_DownloadFiles.keys();

    QStringList l = m_SeleteFilesModel->stringList();
    foreach (const QString &var, list) {
        QString text("[D]" + var);
        if(!l.contains(text)) {
            l <<  text;
        }
    }

    m_SeleteFilesModel->setStringList(l);
}

void FileOperation::slotAddDownloadFiles(const QStringList &downloadFiles)
{
    this->show();
    this->activateWindow();
    foreach (const QString &var, downloadFiles) {
        if(createDownloadTask(var, true)) {
            createDownItemAndProgressTask(var);
        }
    }
}

void FileOperation::slotDownloadClicked()
{
    if(!m_SeleteFilesModel) return;
    m_ThreadPool.setMaxThreadCount(giv_SessionMsg.sessions.value(m_ClientName).uploadNum*2 + giv_SessionMsg.sessions.value(m_ClientName).downloadNum*2);

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
            createDownItemAndProgressTask(QDir::cleanPath(createPath + QDir::separator() + p));
        }
    }

//    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_DownloadFiles.clear();
//    on_pushButton_Cancel_clicked();
}

void FileOperation::slotDownCancel(QString path)
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

    if(m_DownGetProgressTasks.contains(path)) {
        auto item = m_DownGetProgressTasks.value(path);
        if(item) {
            item->stop();
            item->deleteLater();
        }
        m_DownGetProgressTasks.remove(path);
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

void FileOperation::slotDownReset(QString from, QString to)
{
    GoStr path(from);
    GoStr serverPath(to);
    QSharedPointer<char> msg1(DownRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
    if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
        QMessageBox::critical(this, "下载重置情况", QString("下载重置错误(%1)").arg(msg1.data()));
        return;
    }
}

void FileOperation::slotDownloadState(QString path, bool running)
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

    if(m_DownGetProgressTasks.contains(path)) {
        auto item = m_DownGetProgressTasks.value(path);
        if(item) {
            if(running) {
                m_ThreadPool.start(item);
            } else {
                item->stop();
            }
        }
    }
}




