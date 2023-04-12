#include "LocalFileManageMainWin.h"
#include "ui_LocalFileManageMainWin.h"

LocalFileManageMainWin::LocalFileManageMainWin(const QString &clientName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LocalFileManageMainWin)
{
    ui->setupUi(this);

    m_RightClickMenu = nullptr;
    m_ActionPaste = nullptr;
    m_ActionCopy = nullptr;
    m_ActionDelete = nullptr;
    m_ActionCut = nullptr;
    m_ActionRefresh = nullptr;
    m_ActionOpen = nullptr;
    m_ActionRename = nullptr;
    m_LabelMsg = nullptr;
    m_DebugMsgWidget = nullptr;

    createContextMenu();

    m_ClientName = clientName;
    // 设置本地路径
    QString p = giv_SessionMsg.sessions.value(m_ClientName).localPath;
    ui->lineEdit->setText(p);
    ui->tableView->setPath(p);

    // 创建menu--Actions
    m_ActionsStr << "上一级目录" << "刷新" << "打开" << "复制"
                 << "粘贴" << "剪切" << "删除" << "重命名" << "上传";

    for(auto &var : m_ActionsStr) {
        ui->menubar->addAction(var);
    }

    QObject::connect(ui->menubar, &QMenuBar::triggered, this, &LocalFileManageMainWin::slotActionTriggered);

    // model
    m_Model = new QStandardItemModel(ui->tableView);
    ui->tableView->setModel(m_Model);

    connect(ui->tableView, &TableView::sigDropUrls, this, [this](const QList<QUrl> urls, bool copy){
        int row = ui->tableView->currentIndex().row();
        QModelIndex typeIndex = m_Model->index(row, 2);
        QModelIndex nameIndex = m_Model->index(row, 0);

        QString name = "";
        if(typeIndex.data().toString() == "文件夹") {
            name = nameIndex.data().toString();
        }

        for(const auto &url : urls) {
            QString p = url.toLocalFile();
            p = p.mid(p.lastIndexOf('/')+1);
            if(copy) {
                m_LocalFileManageApi.copy(url.toLocalFile(), QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + name + QDir::separator() + p));
            } else {
                m_LocalFileManageApi.move(url.toLocalFile(), QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + name + QDir::separator() + p));
            }
        }
    });

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &LocalFileManageMainWin::on_pushButton_Search_clicked);

    connect(ui->tableView, &TableView::doubleClicked, this, &LocalFileManageMainWin::slotOpen);
    connect(ui->tableView, &TableView::sigClicked, this, &LocalFileManageMainWin::slotSelectedInfo);
    connect(ui->tableView, &TableView::customContextMenuRequested, this, &LocalFileManageMainWin::slotContextRightClickMenu);

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


    // statusBar
    m_LabelMsg = new QLabel(" 请选择项目");
    ui->statusbar->addWidget(m_LabelMsg, 1);
    ui->statusbar->setSizeGripEnabled(false); //不显示右下角拖放控制点

    // QFileSystemWatcher
    m_FileSystemWatcher = new QFileSystemWatcher(this);
    connect(m_FileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &LocalFileManageMainWin::slotRefresh);
    connect(m_FileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LocalFileManageMainWin::slotRefresh);

    setWatchPath(ui->lineEdit->text());
    // 刷新
    slotRefresh();
}

LocalFileManageMainWin::~LocalFileManageMainWin()
{
    if(m_DebugMsgWidget) {
        m_DebugMsgWidget->deleteLater();
    }

    delete ui;
}

void LocalFileManageMainWin::setWatchPath(const QString &path)
{
    if(path.isEmpty()) return;
    // 监控文件或目录
    if (m_FileSystemWatcher) {
        // 添加监控路径
        m_FileSystemWatcher->addPath(path);
    }
}

void LocalFileManageMainWin::on_pushButton_Search_clicked()
{
    QString path = ui->lineEdit->text();
    if(path.isEmpty()) return;

    setWatchPath(path);
    m_LocalFileManageApi.readerDir(path, m_Model);
}

void LocalFileManageMainWin::slotActionTriggered(QAction *action)
{
    if(!action) return;
    QString name = action->text();

//    m_ActionsStr << "上一级目录" << "刷新" << "打开" << "复制" << "粘贴" << "剪切" << "删除" << "重命名" << "上传";
    debugMsg(name);

    switch (m_ActionsStr.indexOf(name)) {
    case 0:
        // 上一级目录
        slotParentDir();
        break;
    case 1:
        // 刷新
        slotRefresh();
        break;
    case 2:
        // 打开
        slotOpen();
        break;
    case 3:
        // 复制
        slotCopy();
        break;
    case 4:
        // 粘贴
        slotPaste();
        break;
    case 5:
        // 剪切
        slotCut();
        break;
    case 6:
        // 删除
        slotDelete();
        break;
    case 7:
        // 重命名
        slotRename();
        break;
    case 8:
        // 上传
        slotUpload();
        break;
    case 9:
        // 上传
        slotDebugWidgetOpen();
        break;
    default:
        break;
    }

    return;
}

void LocalFileManageMainWin::slotParentDir()
{
    QString path = ui->lineEdit->text();
    QFileInfo fileInfo(path);
    QString path1 = fileInfo.dir().absolutePath();
    ui->lineEdit->setText(path1);
    ui->tableView->setPath(path1);

    setWatchPath(path1);

    if(path1.isEmpty()) {
        QMessageBox::information(this, "目录", "路径为空");
        return;
    }

    m_LocalFileManageApi.readerDir(path1, m_Model);

    emit sigDebugMsg(QString("[上一级目录]：%1 >> %2").arg(path).arg(path1));
}

void LocalFileManageMainWin::slotRefresh()
{
    m_LocalFileManageApi.readerDir(ui->lineEdit->text(), m_Model);

    emit sigDebugMsg(QString("[刷新]：%1").arg(ui->lineEdit->text()));
}

void LocalFileManageMainWin::slotOpen()
{
    int row = ui->tableView->currentIndex().row();
    QModelIndex typeIndex = m_Model->index(row, 2);
    QModelIndex nameIndex = m_Model->index(row, 0);
    QString path = QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + nameIndex.data().toString());

    if(typeIndex.data().toString() == "文件夹") {
        m_LocalFileManageApi.readerDir(path, m_Model);
        ui->lineEdit->setText(path);
        ui->tableView->setPath(path);

        setWatchPath(path);

        emit sigDebugMsg(QString("[打开文件夹]：%1").arg(path));
    } else {

        QString &&typeStr = typeIndex.data().toString();
        if(typeStr.contains("CFCDOWNLOAD_FILE") || typeStr.contains("CFCUPLOAD_FILE")) return;
        if(typeStr.contains("CFCDOWNLOAD_INFO")) {
            emit sigAddDownloadFiles(QStringList() << path);
            return;
        }

        if(typeStr.contains("CFCUPLOAD_INFO")) {
            emit sigAddUpFiles(QStringList() << path);
            return;
        }


        QString name(nameIndex.data().toString());
        if(name == "..") {
            slotParentDir();
            return;
        }


        m_LocalFileManageApi.open(path);

        emit sigDebugMsg(QString("[打开文件]：%1").arg(path));
    }
}

void LocalFileManageMainWin::slotCopy()
{
    QModelIndexList l = ui->tableView->selectionModel()->selectedRows();
    m_CopyList.clear();
    for(const auto &var : l) {
        m_CopyList.append(QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + var.data().toString()));
    }
    m_Copy = true;
}

void LocalFileManageMainWin::slotPaste()
{
    for(auto &var : m_CopyList) {
        QFileInfo fileInfo(var);
        if(!fileInfo.exists()) continue;

        if(m_Copy) {
            // 复制
            m_LocalFileManageApi.copy(var,  QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + fileInfo.fileName()));
        } else {
            // 剪切
            m_LocalFileManageApi.move(var,  QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + fileInfo.fileName()));
        }
    }

    if(!m_Copy) m_CopyList.clear();
}

void LocalFileManageMainWin::slotCut()
{
    slotCopy();
    m_Copy = false;
}

void LocalFileManageMainWin::slotDelete()
{
    QModelIndexList l = ui->tableView->selectionModel()->selectedRows();
    for(const auto &var : l) {
        m_LocalFileManageApi.remove(QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + var.data().toString()));
    }
}

void LocalFileManageMainWin::slotRename()
{
    if(!m_Model) return;
    int row = ui->tableView->currentIndex().row();
    auto index = m_Model->index(row, 0);
    QString name = index.data().toString();
    if(name.isEmpty()) return;

    bool ok = false;
    QString text = QInputDialog::getText(this, tr("重命名"), tr(""), QLineEdit::Normal, name, &ok);
    if (ok && !text.isEmpty() && name != text) {
        QString path = QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + name);
        QString path1 = QDir::cleanPath(ui->lineEdit->text() + QDir::separator() + text);

        m_LocalFileManageApi.move(path, path1);

        emit sigDebugMsg(QString("[重命名]:%1 >> %2").arg(path).arg(path1));
    }
}

void LocalFileManageMainWin::slotUpload()
{
    QModelIndexList names = ui->tableView->selectionModel()->selectedRows();
    QModelIndexList types = ui->tableView->selectionModel()->selectedRows(2);
    QString path = ui->lineEdit->text();

    QMap<QString, QString> uploadFiles;
    for(int i = 0; i < types.size(); i++) {
        const QModelIndex type = types.at(i);
        QString p = QDir::cleanPath(path + QDir::separator() + names.at(i).data().toString());
        if(type.data().toString() == "文件夹") {
            uploadFiles.insert(m_LocalFileManageApi.getFilesByDir(p, path));
        } else {
            uploadFiles.insert(p, "");
        }
    }

    emit sigUpload(uploadFiles, ui->lineEdit->text());

    emit sigDebugMsg(tr("执行了上传操作"));
}

// 选择的信息
void LocalFileManageMainWin::slotSelectedInfo(const QPoint &pos)
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

void LocalFileManageMainWin::slotContextRightClickMenu(const QPoint &pos)
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

// 创建菜单
void LocalFileManageMainWin::createContextMenu()
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

void LocalFileManageMainWin::slotDebugWidgetOpen()
{
    if(!m_DebugMsgWidget) {
        m_DebugMsgWidget = new DebugMsgWidget();
        m_DebugMsgWidget->setTitle("本地文件管理打印信息");
        connect(this, &LocalFileManageMainWin::sigDebugMsg, m_DebugMsgWidget, &DebugMsgWidget::slotAppendDebugMsg);
    }

    m_DebugMsgWidget->show();
}



