#include "UploadFileWidget.h"
#include "ui_UploadFileWidget.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/ScanCFCFile.h"
#include "UploadFileItem.h"

UploadFileWidget::UploadFileWidget(int fc, const QString &clientName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UploadFileWidget)
{
    ui->setupUi(this);

    m_fc = fc;
    m_ClientName = clientName;

    this->setWindowTitle("上传");

    m_SeleteFilesModel = new QStringListModel(ui->listView_FileList);
    ui->listView_FileList->setEditTriggers(QListView::NoEditTriggers);			//不能编辑
    ui->listView_FileList->setSelectionBehavior(QListView::SelectRows);		//一次选中整行
    ui->listView_FileList->setSelectionMode(QListView::ExtendedSelection);       //SingleSelection

    ui->listView_FileList->setModel(m_SeleteFilesModel);

    qRegisterMetaType<ProgressContext>("ProgressContext");
}

UploadFileWidget::~UploadFileWidget()
{
    delete ui;
}

void UploadFileWidget::setfc(int fc)
{
    m_fc = fc;
}

bool UploadFileWidget::createUploadTask(QString path, bool buff)
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
    m_UploadTasks.insert(path, asycUploadFile);

    m_ThreadPool.start(asycUploadFile);

    return true;
}

void UploadFileWidget::createItemAndProgressTask(QString path)
{
    path.remove(".CFCUpload_info");

    // 删除获取进度
    if(m_GetProgressTasks.contains(path)) {
        AsycGetProgress *task = m_GetProgressTasks.value(path);
        task->stop();
        task->deleteLater();
    }

    AsycGetProgress *getProgress = new AsycGetProgress(m_fc, path, AsycGetProgress::Upload);
    m_GetProgressTasks.insert(path, getProgress);

    //
    if(m_UploadFileItems.contains(path)) {
        QListWidgetItem *item = m_UploadFileItems.value(path);
        if(item) delete item;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(0, 100));
    UploadFileItem *uploadFileItem = new UploadFileItem(ui->listWidget);
    connect(uploadFileItem, &UploadFileItem::sigCancel, this, &UploadFileWidget::slotCancel);
    connect(uploadFileItem, &UploadFileItem::sigReset, this, &UploadFileWidget::slotReset);
    connect(uploadFileItem, &UploadFileItem::sigUploadState, this, &UploadFileWidget::slotUploadState);

    connect(getProgress, &AsycGetProgress::sigDownloadFileProgress, uploadFileItem, &UploadFileItem::slotDownloadProgress);
    connect(getProgress, &AsycGetProgress::sigUploadFinished, this, &UploadFileWidget::sigUploadFinished);

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, uploadFileItem);
    m_UploadFileItems.insert(path, item);

    // 开始任务
    m_ThreadPool.start(getProgress);
}

void UploadFileWidget::stopUpload(QString path)
{
    path.remove(".CFCUpload_info");

    if(m_UploadTasks.contains(path)) {
        auto item = m_UploadTasks.value(path);
        if(item) {
            item->stopUploadTask();
        }
    }

    if(m_GetProgressTasks.contains(path)) {
        auto item = m_GetProgressTasks.value(path);
        if(item) {
            item->stop();
        }
    }

}

void UploadFileWidget::slotServerPath(const QString &path)
{
    m_ServerPath = path;
}

void UploadFileWidget::slotUploadFilesInfo(const QMap<QString, QString> &paths, bool clear)
{
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Upload->setEnabled(true);

    if(clear) m_UploadFiles.clear();

    m_UploadFiles.insert(paths);
    QStringList &&list = m_UploadFiles.keys();

    m_SeleteFilesModel->setStringList(list);
}

void UploadFileWidget::slotClientPath(const QString &path)
{
    m_ClientPath = path;
}

void UploadFileWidget::on_pushButton_SeleteFile_clicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("上传文件"), m_ClientPath);
    if(paths.isEmpty() || !m_SeleteFilesModel) return;

    ui->pushButton_Upload->setEnabled(true);


    for(const auto &path : paths) {
        QFileInfo fileInfo(path);
        QString p = fileInfo.path();
        p.remove(m_ClientPath);
        m_UploadFiles.insert(path, p);
    }

    QStringList &&list = m_UploadFiles.keys();
    m_SeleteFilesModel->setStringList(list);
}

void UploadFileWidget::on_pushButton_Cancel_clicked()
{
    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_UploadFiles.clear();
    ui->pushButton_Upload->setEnabled(false);
}

void UploadFileWidget::on_pushButton_Upload_clicked()
{
    if(!m_SeleteFilesModel) return;
    m_ThreadPool.setMaxThreadCount(giv_SessionMsg.sessions.value(m_ClientName).uploadNum*2);

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
            createItemAndProgressTask(var);
        }

    }

//    on_pushButton_Cancel_clicked();
    if(m_SeleteFilesModel) m_SeleteFilesModel->setStringList(QStringList());

    m_UploadFiles.clear();
}

void UploadFileWidget::slotAddUploadFiles(const QStringList &upFiles)
{
    this->show();
    this->activateWindow();
    foreach (const QString &var, upFiles) {
        if(createUploadTask(var, true)) {
            createItemAndProgressTask(var);
        }
    }
}

void UploadFileWidget::slotCancel(QString path)
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

    if(m_GetProgressTasks.contains(path)) {
        auto item = m_GetProgressTasks.value(path);
        if(item) {
            item->stop();
            item->deleteLater();
        }
        m_GetProgressTasks.remove(path);
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

void UploadFileWidget::slotReset(QString from, QString to)
{
    GoStr path(from);
    GoStr serverPath(to);
    QSharedPointer<char> msg1(UpRemoteFileContextToNewTask(m_fc, path.getGoString(), serverPath.getGoString(), true, 0));
    if(!QString(msg1.data()).contains("\"ErrMsg\":\"\"")) {
        QMessageBox::critical(this, "上传重置情况", QString("上传重置错误(%1)").arg(msg1.data()));
        return;
    }
}

void UploadFileWidget::slotUploadState(QString path, bool running)
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

void UploadFileWidget::on_pushButton_DeleteItem_clicked()
{
    if(!m_SeleteFilesModel) return;

    QModelIndexList selIndexs = ui->listView_FileList->selectionModel()->selectedRows();
    foreach (const QModelIndex& index, selIndexs) {
        QString key = m_SeleteFilesModel->index(index.row()).data().toString();
        m_SeleteFilesModel->removeRow(index.row());
        m_UploadFiles.remove(key);
    }
}

void UploadFileWidget::on_pushButton_Close_clicked()
{
    this->close();
}

