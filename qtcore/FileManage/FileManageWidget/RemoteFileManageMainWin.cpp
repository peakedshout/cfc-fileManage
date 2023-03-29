#include "RemoteFileManageMainWin.h"
#include "ui_RemoteFileManageMainWin.h"
#include "RewriteApi/GoStr.h"

RemoteFileManageMainWin::RemoteFileManageMainWin(int fc, const QString &serverName, const QString &currPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RemoteFileManageMainWin)
{
    ui->setupUi(this);

    if(fc <= 0) return;
    m_ServerName = serverName;

    m_RightClickMenu = nullptr;
    m_ActionPaste = nullptr;
    m_ActionCopy = nullptr;
    m_ActionDelete = nullptr;
    m_ActionCut = nullptr;
    m_ActionRefresh = nullptr;
    m_ActionOpen = nullptr;
    m_ActionRename = nullptr;
    m_UploadFileWidget = nullptr;
    m_DownloadFileWidget = nullptr;
    m_Model = nullptr;
    m_LabelMsg = nullptr;
    m_DelayMsg = nullptr;
    m_UpSpeedMsg = nullptr;
    m_DownSpeedMsg = nullptr;
    m_AsycNetStatus = nullptr;
    m_SessionMsgDialog = nullptr;
    m_DebugMsgWidget = nullptr;

    m_fc = fc;

    // statusBar
    m_LabelMsg = new QLabel(" 请选择项目");
    ui->statusbar->addWidget(m_LabelMsg, 4);

    QLabel *l = new QLabel();
    l->setPixmap(QPixmap(":/png/upspeed.png").scaled(20,20));
    l->setAlignment(Qt::AlignCenter);
    l->setFixedWidth(20);
    ui->statusbar->addPermanentWidget(l);

    m_UpSpeedMsg = new QLabel("上传");
    m_UpSpeedMsg->setAlignment(Qt::AlignCenter);
    m_UpSpeedMsg->setToolTip("上传：0.00ms");
    ui->statusbar->addPermanentWidget(m_UpSpeedMsg, 1);

    l = new QLabel();
    l->setPixmap(QPixmap(":/png/downloadSpeed.png").scaled(20,20));
    l->setAlignment(Qt::AlignCenter);
    l->setFixedWidth(20);
    ui->statusbar->addPermanentWidget(l);

    m_DownSpeedMsg = new QLabel("下载");
    m_DownSpeedMsg->setAlignment(Qt::AlignCenter);
    m_DownSpeedMsg->setToolTip("下载：0.00ms");
    ui->statusbar->addPermanentWidget(m_DownSpeedMsg, 1);

    l = new QLabel();
    l->setPixmap(QPixmap(":/png/delay.png").scaled(20,20));
    l->setAlignment(Qt::AlignCenter);
    l->setFixedWidth(20);
    ui->statusbar->addPermanentWidget(l);

    m_DelayMsg = new QLabel("延迟");
    m_DelayMsg->setAlignment(Qt::AlignCenter);
    m_DelayMsg->setToolTip("延迟：0.00ms");
    ui->statusbar->addPermanentWidget(m_DelayMsg, 1);

    ui->statusbar->setSizeGripEnabled(false); //不显示右下角拖放控制点



    m_Model = new QStandardItemModel(ui->tableView);
    ui->tableView->setModel(m_Model);

    m_Model->setColumnCount(4);
    m_Model->setHeaderData(0,Qt::Horizontal, "名称");
    m_Model->setHeaderData(1,Qt::Horizontal, "修改日期");
    m_Model->setHeaderData(2,Qt::Horizontal, "类型");
    m_Model->setHeaderData(3,Qt::Horizontal, "大小");

//    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->setColumnWidth(0, 260);
    ui->tableView->setColumnWidth(1, 160);
    ui->tableView->setColumnWidth(2, 90);
    ui->tableView->setColumnWidth(3, 80);

    ui->lineEdit->setText(currPath);
    ui->tableView->setPath(currPath);
    m_RemoteFileManageApi.setFC(m_fc);

    createContextMenu();

    // 创建menu--Actions
    m_ActionsStr << "上一级目录" << "刷新" << "打开" << "复制" << "粘贴" << "剪切" << "删除" <<"重命名"<< "下载" << "用户信息" << "打印信息";

    for(auto &var : m_ActionsStr) {
        ui->menubar->addAction(var);
    }

    connect(ui->menubar, &QMenuBar::triggered, this, &RemoteFileManageMainWin::slotActionTriggered);

    connect(ui->tableView, &TableView::sigDropUrls, this, [this](const QList<QUrl> urls, bool copy){
        int row = ui->tableView->currentIndex().row();
        QModelIndex typeIndex = m_Model->index(row, 2);
        QModelIndex nameIndex = m_Model->index(row, 0);

        QString name = "";
        if(typeIndex.data().toString() == "文件夹") {
            name = nameIndex.data().toString();
        }

        QString path = ui->lineEdit->text();
        if(path == "/") path = "";

        QMap<QString, QString> paths;
        for(const auto &url : urls) {
            QString p = url.toLocalFile();

            p = p.mid(p.lastIndexOf('/')+1);
            if(copy) {
                // 上传
                paths.insert(url.toLocalFile(), "");
            } else {
                // 移动
                if(!name.isEmpty()) name += "/";
                QString fileP = QDir::cleanPath(path + QDir::separator() + name + p);
                if(QDir::cleanPath(url.toLocalFile()) != fileP) {
                    m_RemoteFileManageApi.move(url.toLocalFile(), fileP);
                }
            }
        }
        if(copy) {
            if(!m_UploadFileWidget) {
                m_UploadFileWidget = new UploadFileWidget(m_fc, m_ServerName);
                connect(m_UploadFileWidget, &UploadFileWidget::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
            }
            m_UploadFileWidget->slotClientPath("");
            m_UploadFileWidget->slotServerPath(ui->lineEdit->text());
            m_UploadFileWidget->slotUploadFilesInfo(paths, true);
            m_UploadFileWidget->on_pushButton_Upload_clicked();
        }

        slotRefresh();
    });
    connect(ui->tableView, &TableView::sigDragUrls, this, [this](const QList<QUrl> urls, bool copy){
        // 下载-- 未实现
        Q_UNUSED(this);
        Q_UNUSED(urls);
        Q_UNUSED(copy);

        QMessageBox::information(this, "未完成", "待开发");
    });


    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &RemoteFileManageMainWin::on_pushButton_Search_clicked);

    connect(ui->tableView, &TableView::doubleClicked, this, &RemoteFileManageMainWin::slotOpen);
    connect(ui->tableView, &TableView::sigClicked, this, &RemoteFileManageMainWin::slotSelectedInfo);
    connect(ui->tableView, &TableView::customContextMenuRequested, this, &RemoteFileManageMainWin::slotContextRightClickMenu);

    slotRunNetStatus();

    slotRefresh();
}

RemoteFileManageMainWin::~RemoteFileManageMainWin()
{
    if(m_DownloadFileWidget) {
        delete m_DownloadFileWidget;
    }
    if(m_UploadFileWidget) {
        delete m_UploadFileWidget;
    }

    if(m_AsycNetStatus) {
        m_AsycNetStatus->stop();
    }

    if(m_DebugMsgWidget) {
        m_DebugMsgWidget->deleteLater();
    }

    if(m_fc > 0) {
        CloseFileContext(m_fc);
    }

    delete ui;
}

void RemoteFileManageMainWin::on_pushButton_Search_clicked()
{
    QString path = ui->lineEdit->text();
    if(path.isEmpty()) return;

    QString curPath;
    m_RemoteFileManageApi.readerDir(path, m_Model, curPath);
    ui->lineEdit->setText(curPath);
    ui->tableView->setPath(curPath);

}

void RemoteFileManageMainWin::slotActionTriggered(QAction *action)
{

    if(!action) return;
    QString name = action->text();
    if(!m_ActionsStr.contains(name)) return;
//    m_ActionsStr << "上一级目录" << "刷新" << "打开" << "复制" << "粘贴"
//            << "剪切" << "删除" <<"重命名"<< "下载" << "用户信息" << "打印信息";

    debugMsg(name);
    if(name == m_ActionsStr.at(0)) {
        // 上一级目录
        slotParentDir();
    } else if(name == m_ActionsStr.at(1)) {
        // 刷新
        slotRefresh();
    } else if(name == m_ActionsStr.at(2)) {
        // 打开
        slotOpen();
    } else if(name == m_ActionsStr.at(3)) {
        // 复制
        slotCopy();
    } else if(name == m_ActionsStr.at(4)) {
        // 粘贴
        slotPaste();
    } else if(name == m_ActionsStr.at(5)) {
        // 剪切
        slotCut();
    } else if(name == m_ActionsStr.at(6)) {
        // 删除
        slotDelete();
    } else if(name == m_ActionsStr.at(7)) {
        // 重命名
        slotRename();
    } else if(name == m_ActionsStr.at(8)) {
        // 下载
        slotDownload();
    } else if(name == m_ActionsStr.at(9)) {
        // 用户信息
        slotSessionMsgDialogOpen();
    }
    else if(name == m_ActionsStr.at(10)) {
        // 打印信息
        slotDebugWidgetOpen();
    }


}

void RemoteFileManageMainWin::slotParentDir()
{
    QString path = ui->lineEdit->text();
    if(path == "/") return;

    QFileInfo fileInfo(path);
    QString path1 = fileInfo.dir().path();


    if(path1.isEmpty()) {
        QMessageBox::information(this, "目录", "路径为空");
        return;
    }

    QString curPath;
    m_RemoteFileManageApi.readerDir(path1, m_Model, curPath);
    ui->lineEdit->setText(curPath);
    ui->tableView->setPath(curPath);

    emit sigDebugMsg(QString("[上一级目录]：%1 >> %2").arg(path).arg(ui->lineEdit->text()));
}

void RemoteFileManageMainWin::slotRefresh()
{
    QString curPath;
    m_RemoteFileManageApi.readerDir(ui->lineEdit->text(), m_Model, curPath);
    ui->lineEdit->setText(curPath);
    ui->tableView->setPath(curPath);


    emit sigDebugMsg(QString("[刷新]：%1").arg(ui->lineEdit->text()));
}

void RemoteFileManageMainWin::slotOpen()
{
    int row = ui->tableView->currentIndex().row();
    QModelIndex typeIndex = m_Model->index(row, 2);
    QModelIndex nameIndex = m_Model->index(row, 0);

    QString p = ui->lineEdit->text();
    if(p == "/") p = "";

    QString path = QDir::cleanPath(p + QDir::separator() + nameIndex.data().toString());

    if(typeIndex.data().toString() == "文件夹") {

        QString curPath;
        m_RemoteFileManageApi.readerDir(path, m_Model, curPath);
        ui->lineEdit->setText(curPath);
        ui->tableView->setPath(curPath);

        emit sigDebugMsg(QString("[打开文件夹]：%1").arg(path));
    } else {
//        m_RemoteFileManageApi.open(path);
        QMessageBox::information(this, "未完成", "待开发");
//        emit sigDebugMsg(QString("[打开文件]：%1").arg(path));
    }
}

void RemoteFileManageMainWin::slotCopy()
{
    QModelIndexList l = ui->tableView->selectionModel()->selectedRows();
    m_CopyList.clear();

    QString path = ui->lineEdit->text();
    if(path == "/") path = "";

    for(const auto &var : l) {
        m_CopyList.append(QDir::cleanPath(path + QDir::separator() + var.data().toString()));
    }
    m_Copy = true;
}

void RemoteFileManageMainWin::slotPaste()
{
    for(auto &var : m_CopyList) {
        QFileInfo fileInfo(var);
        QString path = ui->lineEdit->text();
        if(path == "/") path = "";
        if(m_Copy) {
            // 复制
            m_RemoteFileManageApi.copy(var,  QDir::cleanPath(path + QDir::separator() + fileInfo.fileName()));
        } else {
            // 剪切
            m_RemoteFileManageApi.move(var,  QDir::cleanPath(path + QDir::separator() + fileInfo.fileName()));
        }
    }

    if(!m_Copy) m_CopyList.clear();

    slotRefresh();
}

void RemoteFileManageMainWin::slotCut()
{
    slotCopy();
    m_Copy = false;
}

void RemoteFileManageMainWin::slotDelete()
{
    QModelIndexList l = ui->tableView->selectionModel()->selectedRows();

    QString path = ui->lineEdit->text();
    if(path == "/") path = "";

    for(const auto &var : l) {
        m_RemoteFileManageApi.remove(QDir::cleanPath(path + QDir::separator() + var.data().toString()));
    }

    slotRefresh();
}

void RemoteFileManageMainWin::slotRename()
{
    if(!m_Model) return;
    int row = ui->tableView->currentIndex().row();
    auto index = m_Model->index(row, 0);
    QString name = index.data().toString();
    if(name.isEmpty()) return;

    bool ok = false;
    QString text = QInputDialog::getText(this, tr("重命名"), tr(""), QLineEdit::Normal, name, &ok);
    if (ok && !text.isEmpty() && name != text) {
        QString p = ui->lineEdit->text();
        if(p == "/") p = "";
        QString path = QDir::cleanPath(p + QDir::separator() + name);
        QString path1 = QDir::cleanPath(p + QDir::separator() + text);

        m_RemoteFileManageApi.move(path, path1);

        slotRefresh();

        emit sigDebugMsg(QString("[重命名]:%1 >> %2").arg(path).arg(path1));
    }

}

void RemoteFileManageMainWin::slotUpload(const QMap<QString, QString> &uploadFiles, const QString &clientPath)
{
    if(!m_UploadFileWidget) {
        m_UploadFileWidget = new UploadFileWidget(m_fc, m_ServerName);
        connect(m_UploadFileWidget, &UploadFileWidget::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
    }

    m_UploadFileWidget->slotClientPath(clientPath);
    m_UploadFileWidget->slotServerPath(ui->lineEdit->text());
    m_UploadFileWidget->slotUploadFilesInfo(uploadFiles);

    m_UploadFileWidget->show();
}

void RemoteFileManageMainWin::slotDownload()
{
    if(!m_DownloadFileWidget) {
        m_DownloadFileWidget = new DownloadFileWidget(m_fc, m_ServerName);
    }

    QModelIndexList names = ui->tableView->selectionModel()->selectedRows();
    QModelIndexList types = ui->tableView->selectionModel()->selectedRows(2);
    QString path = ui->lineEdit->text();
    if(path == "/") path = "";

    QMap<QString, QString> downloadFiles;
    for(int i = 0; i < types.size(); i++) {
        const QModelIndex type = types.at(i);
        QString p = QDir::cleanPath(path + QDir::separator() + names.at(i).data().toString());
        if(type.data().toString() == "文件夹") {
            downloadFiles.insert(m_RemoteFileManageApi.getFilesByDir(p, ui->lineEdit->text()));
        } else {
            downloadFiles.insert(p, "");
        }
    }

    QString p = giv_SessionMsg.sessions.value(m_ServerName).savePath;
    m_DownloadFileWidget->slotClientPath(p);
    m_DownloadFileWidget->slotServerPath(p);
    m_DownloadFileWidget->slotDownloadFilesInfo(downloadFiles);

    emit sigDebugMsg(tr("执行了下载操作"));
    m_DownloadFileWidget->show();
}

// 选择的信息
void RemoteFileManageMainWin::slotSelectedInfo(const QPoint &pos)
{
    if(!m_Model) return;

    QModelIndex index = ui->tableView->indexAt(pos);
    int count = 0;
    int sum = m_Model->rowCount();

    if(index.isValid()) {
        QModelIndexList selecteds = ui->tableView->selectionModel()->selectedRows();
        count = selecteds.size();
    }


    QString msg = QString(" %1 个项目  选中 %2 个项目").arg(sum).arg(count);
    m_LabelMsg->setText(msg);

    emit sigDebugMsg(msg);
}

void RemoteFileManageMainWin::slotContextRightClickMenu(const QPoint &pos)
{
    if(!m_RightClickMenu) return;
    auto index = ui->tableView->indexAt(pos);
    if (index.isValid())
    {
        if(m_ActionOpen) m_ActionOpen->setEnabled(true);
        if(m_ActionCopy) m_ActionCopy->setEnabled(true);
        if(m_ActionRename) m_ActionRename->setEnabled(true);
        if(m_ActionCut) m_ActionCut->setEnabled(true);
        if(m_ActionDelete) m_ActionDelete->setEnabled(true);
    } else{
        if(m_ActionOpen) m_ActionOpen->setEnabled(false);
        if(m_ActionCopy) m_ActionCopy->setEnabled(false);
        if(m_ActionRename) m_ActionRename->setEnabled(false);
        if(m_ActionCut) m_ActionCut->setEnabled(false);
        if(m_ActionDelete) m_ActionDelete->setEnabled(false);
    }
    m_RightClickMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
}

void RemoteFileManageMainWin::slotRunNetStatus()
{
    if(!m_AsycNetStatus) {
        m_AsycNetStatus = new AsycNetStatus(m_fc);
        if(m_DelayMsg) {
            connect(m_AsycNetStatus, &AsycNetStatus::sigDelay, this, [this](QString delay){
                m_DelayMsg->setText(delay);
                m_DelayMsg->setToolTip("延迟：" + delay);
            });
        }

        if(m_UpSpeedMsg && m_DownSpeedMsg) {
            connect(m_AsycNetStatus, &AsycNetStatus::sigNetworkSpeed, this, [this](quint64 uploadSpeed, QString uploadSpeedText, quint64 downloadSpeed, QString downloadSpeedText){
                Q_UNUSED(uploadSpeed);
                Q_UNUSED(downloadSpeed);
                m_UpSpeedMsg->setText(uploadSpeedText);
                m_UpSpeedMsg->setToolTip("上传：" + uploadSpeedText);

                m_DownSpeedMsg->setText(downloadSpeedText);
                m_DownSpeedMsg->setToolTip("下载：" + downloadSpeedText);
            });
        }

        if(!m_SessionMsgDialog) {
            m_SessionMsgDialog = new SessionMsgDialog(m_ServerName, this);
            connect(m_SessionMsgDialog, &SessionMsgDialog::sigQuit, this, &RemoteFileManageMainWin::sigQuit);
            connect(m_SessionMsgDialog, &SessionMsgDialog::sigCreate, this, &RemoteFileManageMainWin::sigCreate);
        }
        connect(m_AsycNetStatus, &AsycNetStatus::sigMemoryCapacity, m_SessionMsgDialog, &SessionMsgDialog::slotMemoryCapacity);
    }

    QThreadPool::globalInstance()->start(m_AsycNetStatus);

}

void RemoteFileManageMainWin::slotSessionMsgDialogOpen()
{
    if(!m_SessionMsgDialog) {
        m_SessionMsgDialog = new SessionMsgDialog(m_ServerName, this);

        connect(m_SessionMsgDialog, &SessionMsgDialog::sigQuit, this, &RemoteFileManageMainWin::sigQuit);
        connect(m_SessionMsgDialog, &SessionMsgDialog::sigCreate, this, &RemoteFileManageMainWin::sigCreate);
    }

    m_SessionMsgDialog->exec();
}

void RemoteFileManageMainWin::slotDebugWidgetOpen()
{
    if(!m_DebugMsgWidget) {
        m_DebugMsgWidget = new DebugMsgWidget();
        m_DebugMsgWidget->setTitle("远程文件管理打印信息");
        connect(this, &RemoteFileManageMainWin::sigDebugMsg, m_DebugMsgWidget, &DebugMsgWidget::slotAppendDebugMsg);
    }

    m_DebugMsgWidget->show();
}

// 创建菜单
void RemoteFileManageMainWin::createContextMenu()
{
    if(m_RightClickMenu) return;
    m_RightClickMenu = new QMenu(this);
    m_ActionOpen = m_RightClickMenu->addAction("打开", this, SLOT(slotOpen()));
    m_ActionRefresh = m_RightClickMenu->addAction("刷新", this, SLOT(slotRefresh()));
    m_ActionCopy = m_RightClickMenu->addAction("复制", this, SLOT(slotCopy()));
    m_ActionPaste = m_RightClickMenu->addAction("粘贴", this, SLOT(slotPaste()));
    m_ActionCut = m_RightClickMenu->addAction("剪切", this, SLOT(slotCut()));
    m_ActionRename = m_RightClickMenu->addAction("重命名", this, SLOT(slotRename()));
    m_ActionDelete = m_RightClickMenu->addAction("删除", this, SLOT(slotDelete()));
}





