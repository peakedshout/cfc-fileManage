#include "RemoteFileManageMainWin.h"
#include "ui_RemoteFileManageMainWin.h"
#include "RewriteApi/GoStr.h"
#include "AsycTask/AsycReadFile.h"

RemoteFileManageMainWin::RemoteFileManageMainWin(int fc, const QString &clientName, const QString &currPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RemoteFileManageMainWin)
{
    ui->setupUi(this);

    if(fc <= 0) return;
    m_ClientName = clientName;

    m_RightClickMenu = nullptr;
    m_ActionPaste = nullptr;
    m_ActionCopy = nullptr;
    m_ActionDelete = nullptr;
    m_ActionCut = nullptr;
    m_ActionRefresh = nullptr;
    m_ActionOpen = nullptr;
    m_ActionRename = nullptr;
    m_FileOperation = nullptr;
    m_Model = nullptr;
    m_LabelMsg = nullptr;
    m_DelayMsg = nullptr;
    m_UpSpeedMsg = nullptr;
    m_DownSpeedMsg = nullptr;
    m_AsycNetStatus = nullptr;
    m_SessionMsgDialog = nullptr;
    m_DebugMsgWidget = nullptr;
    m_ScanCFCFileWidget = nullptr;
    m_CountDown = nullptr;
    m_fc = fc;
    m_ISExit_C = true;

    // statusBar
    m_LabelMsg = new QLabel(" 请选择项目");
    ui->statusbar->addWidget(m_LabelMsg, 4);

    QLabel *l = new QLabel();
    l->setPixmap(QPixmap(":/png/up.png").scaled(20,20));
    l->setAlignment(Qt::AlignCenter);
    l->setFixedWidth(20);
    ui->statusbar->addPermanentWidget(l);

    m_UpSpeedMsg = new QLabel("上传");
    m_UpSpeedMsg->setAlignment(Qt::AlignCenter);
    m_UpSpeedMsg->setToolTip("上传：0.00ms");
    ui->statusbar->addPermanentWidget(m_UpSpeedMsg, 1);

    l = new QLabel();
    l->setPixmap(QPixmap(":/png/down.png").scaled(20,20));
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
    m_ActionsStr << "上一级目录" << "刷新" << "打开" << "复制" << "粘贴" << "剪切" << "删除" <<"重命名"<< "下载" << "扫描" << "用户信息";

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
            if(!m_FileOperation) {
                m_FileOperation = new FileOperation(m_fc, m_ClientName);
                connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
                connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
            }
            m_FileOperation->slotClientPath("");
            m_FileOperation->slotServerPath(ui->lineEdit->text());
            m_FileOperation->slotUploadFilesInfo(paths, true);
            m_FileOperation->slotUploadClicked();
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

    m_SDReConn = false;
}

RemoteFileManageMainWin::~RemoteFileManageMainWin()
{
    if(m_CountDown) {
        m_CountDown->deleteLater();
    }

    if(m_FileOperation) {
        m_FileOperation->deleteLater();
    }

    if(m_FileOperation) {
        delete m_FileOperation;
    }

    if(m_AsycNetStatus) {
        m_AsycNetStatus->stop();
    }

    if(m_DebugMsgWidget) {
        m_DebugMsgWidget->deleteLater();
    }

    if(m_ScanCFCFileWidget) {
        m_ScanCFCFileWidget->deleteLater();
    }

    if(m_fc > 0) {
        CloseFileContext(m_fc);
    }


    delete ui;
}

void RemoteFileManageMainWin::setfc(int fc)
{
    m_fc = fc;
    flag = false;
    m_ISExit_C = true;

//    slotRefresh();
    m_RemoteFileManageApi.setFC(m_fc);
    if(m_FileOperation) m_FileOperation->setfc(m_fc);
    if(m_AsycNetStatus) m_AsycNetStatus->setfc(m_fc);
    if(m_ScanCFCFileWidget) m_ScanCFCFileWidget->setfc(m_fc);
//    if(m_CountDown) {
//        QMetaObject::invokeMethod(m_CountDown, "hide", Qt::QueuedConnection);
//        m_CountDown->hide();
//    }


    if(m_SDReConn) {
        m_SDReConn = false;
        QMessageBox::information(this, "重连信息", "重新连接成功");
    }
//    slotRefresh();
    QMetaObject::invokeMethod(this, "slotRefresh", Qt::QueuedConnection);
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
//            << "剪切" << "删除" <<"重命名"<< "下载"  << "扫描" << "用户信息" << "打印信息" << "重连";

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
        //  << "扫描"
        slotScanDialogOpen();
    } else if(name == m_ActionsStr.at(10)) {
        // 用户信息
        slotSessionMsgDialogOpen();
    }  else if(name == m_ActionsStr.at(11)) {
        // 重连
        pushButtonReConn_clicked();
    } else if(name == m_ActionsStr.at(12)) {
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
    flag = true;
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
    QString name(nameIndex.data().toString());
    if(name == "..") {
        slotParentDir();
        return;
    }
    QString p = ui->lineEdit->text();
    if(p == "/") p = "";

    QString path = QDir::cleanPath(p + QDir::separator() + name);

    if(typeIndex.data().toString() == "文件夹") {

        QString curPath;
        m_RemoteFileManageApi.readerDir(path, m_Model, curPath);
        ui->lineEdit->setText(curPath);
        ui->tableView->setPath(curPath);

        emit sigDebugMsg(QString("[打开文件夹]：%1").arg(path));
    } else {

//        m_RemoteFileManageApi.open(path);
        QMessageBox::information(this, "未完成", "待开发");
        return;


        QModelIndex sizeIndex = m_Model->index(row, 3);
        QString sizeStr = sizeIndex.data().toString();

        emit sigDebugMsg(QString("[打开文件]：%1").arg(path));


        QDialog dialog;
        dialog.resize(600, 500);
        dialog.setWindowTitle("浏览");
        dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        float size = 0;

        if(sizeStr.contains("KB")) {
            sizeStr.remove("KB");
            size = sizeStr.toFloat()*1024;
        } else if(sizeStr.contains("MB")) {
            sizeStr.remove("MB");
            size = sizeStr.toFloat()*1024*1024;
        } else if(sizeStr.contains("GB")) {
            sizeStr.remove("GB");
            size = sizeStr.toFloat()*1024*1024*1024;
        } else if(sizeStr.contains("B")) {
            sizeStr.remove("B");
            size = sizeStr.toFloat();
        } else {
            size = sizeStr.toFloat();
        }

        QPlainTextEdit textEdit;
        textEdit.setReadOnly(true);
        QFont font = this->font();
        font.setPointSize(12);
        textEdit.setFont(font);


        AsycReadFile *task = new AsycReadFile(m_fc, path, 0, size);
        connect(task, &AsycReadFile::sigText, &textEdit, &QPlainTextEdit::appendPlainText);

        QThreadPool::globalInstance()->start(task);

        QHBoxLayout HBoxLayout(&dialog);
        HBoxLayout.addWidget(&textEdit);

        dialog.exec();


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
    if(!m_FileOperation) {
        m_FileOperation = new FileOperation(m_fc, m_ClientName);
        connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
        connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
    }

    m_FileOperation->slotClientPath(clientPath);
    m_FileOperation->slotServerPath(ui->lineEdit->text());
    m_FileOperation->slotUploadFilesInfo(uploadFiles);

    m_FileOperation->exec();

}

void RemoteFileManageMainWin::slotAddDownloadFiles(const QStringList &list)
{
    if(!m_FileOperation) {
        m_FileOperation = new FileOperation(m_fc, m_ClientName);
        connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
        connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
    }
    m_FileOperation->slotAddDownloadFiles(list);
    m_FileOperation->exec();
}

void RemoteFileManageMainWin::slotAddUploadFiles(const QStringList &list)
{
    if(!m_FileOperation) {
        m_FileOperation = new FileOperation(m_fc, m_ClientName);
        connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
        connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
    }
    m_FileOperation->slotAddUploadFiles(list);
    m_FileOperation->exec();
}

void RemoteFileManageMainWin::slotDownload()
{
    if(!m_FileOperation) {
        m_FileOperation = new FileOperation(m_fc, m_ClientName);
        connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
        connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
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

    QString p = giv_SessionMsg.sessions.value(m_ClientName).savePath;
    m_FileOperation->slotClientPath(p);
    m_FileOperation->slotServerPath(p);
    m_FileOperation->slotDownloadFilesInfo(downloadFiles);

    emit sigDebugMsg(tr("执行了下载操作"));
    m_FileOperation->exec();
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
            connect(m_AsycNetStatus, &AsycNetStatus::sigNetworkSpeed, this, [this](quint64 uploadSpeed, QString uploadSpeedText, quint64 downloadSpeed, QString downloadSpeedText, bool show){
                Q_UNUSED(uploadSpeed);
                Q_UNUSED(downloadSpeed);
                m_UpSpeedMsg->setText(uploadSpeedText);
                m_UpSpeedMsg->setToolTip("上传：" + uploadSpeedText);

                m_DownSpeedMsg->setText(downloadSpeedText);
                m_DownSpeedMsg->setToolTip("下载：" + downloadSpeedText);

                if(show && m_CountDown) {
                    m_CountDown->hide();
                }
            });
        }

        if(!m_SessionMsgDialog) {
            m_SessionMsgDialog = new SessionMsgDialog(m_ClientName, this);
            connect(m_SessionMsgDialog, &SessionMsgDialog::sigQuit, this, &RemoteFileManageMainWin::sigQuit);
            connect(m_SessionMsgDialog, &SessionMsgDialog::sigCreate, this, &RemoteFileManageMainWin::sigCreate);
        }
        connect(m_AsycNetStatus, &AsycNetStatus::sigMemoryCapacity, m_SessionMsgDialog, &SessionMsgDialog::slotMemoryCapacity);

        if(!m_CountDown) {
            m_CountDown = new CountDown(ui->tableView);
            m_CountDown->hide();
            connect(m_CountDown, &CountDown::sigReConnection, this, &RemoteFileManageMainWin::pushButtonReConn_clicked);
            connect(m_CountDown, &CountDown::sigNoReConn, m_AsycNetStatus, &AsycNetStatus::slotNoReConn);
        }

//        m_AsycNetStatus->setCountDown(m_CountDown);

        connect(m_AsycNetStatus, &AsycNetStatus::sigReConnection, this, [this](int num, bool isExit_C, bool vaild){

            if(!flag) {
                flag = true;
                return;
            }
            m_CountDown->setGeometry(ui->tableView->geometry());
            m_CountDown->show();
            m_CountDown->slotLCDNum(num);

            m_Model->setRowCount(0);

            m_ISExit_C = isExit_C;
            if(vaild) {
                emit sigReConnection(isExit_C);
            }
        });


    }

    QThreadPool::globalInstance()->start(m_AsycNetStatus);

}

void RemoteFileManageMainWin::slotScanDialogOpen()
{
    if(!m_ScanCFCFileWidget) {
        m_ScanCFCFileWidget = new ScanCFCFileWidget(m_fc, giv_SessionMsg.sessions.value(m_ClientName).scanPath);

        if(!m_FileOperation) {
            m_FileOperation = new FileOperation(m_fc, m_ClientName);
            connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
            connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
        }
        connect(m_ScanCFCFileWidget, &ScanCFCFileWidget::sigScanDownFiles, m_FileOperation, &FileOperation::slotAddDownloadFiles);
        connect(m_ScanCFCFileWidget, &ScanCFCFileWidget::sigScanUpFiles, m_FileOperation, &FileOperation::slotAddUploadFiles);
    }

    m_ScanCFCFileWidget->on_pushButton_Scan_clicked();
    m_ScanCFCFileWidget->exec();
//    m_ScanCFCFileWidget->activateWindow();

}

void RemoteFileManageMainWin::slotSessionMsgDialogOpen()
{
    if(!m_SessionMsgDialog) {
        m_SessionMsgDialog = new SessionMsgDialog(m_ClientName, this);

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
    m_DebugMsgWidget->activateWindow();
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

void RemoteFileManageMainWin::slotFileOperationOpen()
{
    if(!m_FileOperation) {
        m_FileOperation = new FileOperation(m_fc, m_ClientName);
        connect(m_FileOperation, &FileOperation::sigUploadFinished, this, &RemoteFileManageMainWin::slotRefresh);
        connect(m_FileOperation, &FileOperation::sigScanFiles, this, &RemoteFileManageMainWin::slotScanDialogOpen);
    }

    m_FileOperation->exec();
}


void RemoteFileManageMainWin::pushButtonReConn_clicked()
{
    m_SDReConn = true;
    emit sigReConnection(m_ISExit_C);
}


void RemoteFileManageMainWin::pushButton_fq_clicked()
{
    emit sigNoReConn();
}

