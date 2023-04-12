#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include<QWindow>
#include<QPainter>
#include <QDrag>
#include <QHeaderView>

//#define  TABLEVIEW_DEBUG

#ifdef  TABLEVIEW_DEBUG
#include <QDebug>
#define debugMsg(msg) qDebug()<< "-----------------[Debug]-----------------\nFile: " << __FILE__ << "\nFunc: " << __FUNCTION__ << "\nLine: " << __LINE__ << "\n--[Msg]--> " << msg << "\n-----------------[Debug/]-----------------\n"
#else
#define debugMsg(msg)
#endif

class TableView : public QTableView
{
    Q_OBJECT
public:
    TableView(QWidget *parent = nullptr);

    void setPath(const QString &path);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

signals:
    void sigDropUrls(const QList<QUrl> &urls, bool copy = true);
    void sigDragUrls(const QList<QUrl> &urls, bool copy = true);

    void sigClicked(const QPoint &pos);
private:
//    QPoint m_StartPos;
    bool m_Copy;

    QString m_Path;
};

#endif // TABLEVIEW_H
