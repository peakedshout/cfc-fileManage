#ifndef UPLOADFILEITEM_H
#define UPLOADFILEITEM_H

#include <QFrame>
#include "RewriteApi/CFCFileApiParse.h"

namespace Ui {
class UploadFileItem;
}

class UploadFileItem : public QFrame
{
    Q_OBJECT

public:
    explicit UploadFileItem(QWidget *parent = nullptr);
    ~UploadFileItem();

public slots:
    void slotDownloadProgress(ProgressContext progressInfo, bool finished);
    void slotBtnState(bool running);

private slots:
    void on_pushButton_switch_clicked();
    void on_pushButton_cancel_clicked();

signals:
    void sigUploadState(const QString &path, bool running);
    void sigCancel(const QString &path);
private:
    Ui::UploadFileItem *ui;

    bool m_BtnState;
};

#endif // UPLOADFILEITEM_H
