#ifndef SCANCFCFILEWIDGET_H
#define SCANCFCFILEWIDGET_H

#include <QDialog>
#include <QFileDialog>
#include <QStringListModel>
#include "SessionMsg/SessionMsg.h"

extern SessionMsg giv_SessionMsg;

namespace Ui {
class ScanCFCFileWidget;
}

class ScanCFCFileWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ScanCFCFileWidget(int fc, const QString &path, QWidget *parent = nullptr);
    ~ScanCFCFileWidget();

    void setScanPath(const QString &path);

    void setfc(int fc);
    void on_pushButton_Scan_clicked();
private slots:
    void on_pushButton_SeleteDir_clicked();



    void on_pushButton_AddList_clicked();

    void on_pushButton_Del_clicked();

    void on_pushButton_Close_clicked();

signals:
    void sigScanUpFiles(const QStringList &uploadFiles);
    void sigScanDownFiles(const QStringList &downFiles);

private:
    Ui::ScanCFCFileWidget *ui;

    int m_fc;
    QStringListModel *m_SeleteFilesModel;
};

#endif // SCANCFCFILEWIDGET_H
