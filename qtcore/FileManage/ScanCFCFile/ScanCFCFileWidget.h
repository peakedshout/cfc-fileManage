#ifndef SCANCFCFILEWIDGET_H
#define SCANCFCFILEWIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QStringListModel>

namespace Ui {
class ScanCFCFileWidget;
}

class ScanCFCFileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScanCFCFileWidget(int fc, QWidget *parent = nullptr);
    ScanCFCFileWidget(int fc, const QString &path, QWidget *parent = nullptr);
    ~ScanCFCFileWidget();

    void setScanPath(const QString &path);

private slots:
    void on_pushButton_SeleteDir_clicked();

    void on_pushButton_Scan_clicked();

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
