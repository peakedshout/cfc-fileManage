#include "TableView.h"
#include <QApplication>

TableView::TableView(QWidget *parent) : QTableView(parent)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setMouseTracking(true);
    setDragDropOverwriteMode(true);


    //设置各种属性
    verticalHeader()->hide();	//隐藏行头
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader()->setSelectionMode(QTableView::NoSelection);
    verticalHeader()->setDefaultSectionSize(45);         //默认列高度



    setIconSize(QSize(45,45));
    setEditTriggers(QTableView::NoEditTriggers);			//不能编辑
    setSelectionBehavior(QTableView::SelectRows);		//一次选中整行
    setSelectionMode(QTableView::ExtendedSelection);       //SingleSelection
    setShowGrid(false);                                  //去掉网格线
    setFocusPolicy(Qt::NoFocus);                         //去掉item选中时虚线框
    setContextMenuPolicy(Qt::CustomContextMenu); //可弹出右键菜单
    setStyleSheet("QHeaderView {\
                        font: bold 10pt;\
                        border: 0px solid rgb(144, 144, 144);\
                        border:0px solid rgb(191,191,191);\
                        border-left-color: rgba(255, 255, 255, 0);\
                        border-top-color: rgba(255, 255, 255, 0);\
                        border-radius:0px;\
                        min-height:29px;\
                    }");

    m_Copy = true;
}

void TableView::setPath(const QString &path)
{
    m_Path = path;
}
//根据鼠标事件开启拖拽
void TableView::mousePressEvent(QMouseEvent *e)
{
//    if (e->buttons() & Qt::LeftButton)
//    {
//        m_StartPos = e->pos();
//    }
    emit sigClicked(e->pos());
    QTableView::mousePressEvent(e);
}

void TableView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;

        QModelIndexList list = this->selectionModel()->selectedRows();

        if(m_Path == "/") m_Path = "";
        for(const auto &index : list) {
            urls.append(QUrl::fromLocalFile(m_Path + "/" + index.data().toString()));
        }

        mimeData->setUrls(urls);
        drag->setMimeData(mimeData);

        // 设置拖拽图片
        QPixmap drag_img(":/png/File.png");
        QPainter painter(&drag_img);
        drag->setPixmap(drag_img);
        drag->setHotSpot(QPoint(drag_img.size().width(),  drag_img.size().height()));

        //***注意：此句会阻塞，进入drag的拖拽消息循环，会依次触发dragEnterEvent、dragMoveEvent、dropEvent消息
        if(drag->exec(Qt::CopyAction) == Qt::CopyAction) {
            emit sigDragUrls(urls);
            debugMsg("drag->exec(Qt::CopyAction) == Qt::CopyAction");
        }
    }
    QTableView::mouseMoveEvent(e);
}

void TableView::mouseReleaseEvent(QMouseEvent *e)
{
    QTableView::mouseReleaseEvent(e);
}

// 拖拽响应事件
void TableView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("text/uri-list"))
    {
        e->acceptProposedAction();
    }
    else
    {
        e->ignore();
    }
}

void TableView::dragMoveEvent(QDragMoveEvent *e)
{
    QModelIndex index = indexAt(e->pos());
    if(index.isValid()) {
        this->setCurrentIndex(index);
        m_Copy = false;
    }
}

void TableView::dropEvent(QDropEvent *e)
{
    // 放下事件
    const QMimeData *mimeData = e->mimeData();  // 获取MIME数据

    // 将其中的第一个URL表示为本地文件路径
//    foreach (auto var, mimeData->urls()) {
//        debugMsg(var.toLocalFile());
//    }

    if(mimeData->hasFormat("text/uri-list"))  // 如果数据中包含URL        if(mimeData->hasUrls())
    {
        // 从外到里
        QList<QUrl> urls = mimeData->urls();  // 获取URL列表
        debugMsg("mimeData->hasFormat()");
        emit sigDropUrls(urls, m_Copy);
        m_Copy = true;



    }

    QTableView::dropEvent(e);
}
