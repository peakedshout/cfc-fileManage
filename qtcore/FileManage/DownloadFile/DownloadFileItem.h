#ifndef DOWNLOADFILEITEM_H
#define DOWNLOADFILEITEM_H

#include <QFrame>
#include "RewriteApi/CFCFileApiParse.h"

namespace Ui {
class DownloadFileItem;
}

class DownloadFileItem : public QFrame
{
    Q_OBJECT

public:
    explicit DownloadFileItem(QWidget *parent = nullptr);
    ~DownloadFileItem();

public slots:
    void slotDownloadProgress(ProgressContext progressInfo, bool finished);
    void slotBtnState(bool running);
    void slotFileFrom(const QString &from);
    void slotFileTo(const QString &to);
private slots:
    void on_pushButton_switch_clicked();
    void on_pushButton_cancel_clicked();

signals:
    void sigDownloadState(const QString &path, bool running);
    void sigCancel(const QString &path);
private:
    Ui::DownloadFileItem *ui;

    bool m_BtnState;
};

#endif // DOWNLOADFILEITEM_H
