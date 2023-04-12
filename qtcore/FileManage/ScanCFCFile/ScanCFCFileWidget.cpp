#include "ScanCFCFileWidget.h"
#include "ui_ScanCFCFileWidget.h"
#include "RewriteApi/ScanCFCFile.h"

ScanCFCFileWidget::ScanCFCFileWidget(int fc, const QString &path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanCFCFileWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("扫描");
    m_fc = fc;
    ui->lineEdit->setText(path);

    m_SeleteFilesModel = new QStringListModel(ui->listView);
    ui->listView->setEditTriggers(QListView::NoEditTriggers);			//不能编辑
    ui->listView->setSelectionBehavior(QListView::SelectRows);		//一次选中整行
    ui->listView->setSelectionMode(QListView::ExtendedSelection);       //SingleSelection


    ui->listView->setModel(m_SeleteFilesModel);
}

ScanCFCFileWidget::~ScanCFCFileWidget()
{
    delete ui;
}

void ScanCFCFileWidget::setScanPath(const QString &path)
{
    ui->lineEdit->setText(path);
}

void ScanCFCFileWidget::setfc(int fc)
{
    m_fc = fc;
}

void ScanCFCFileWidget::on_pushButton_SeleteDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("扫描目录"), ui->lineEdit->text());

    if(path.isEmpty()) return;

    ui->lineEdit->setText(path);
}


void ScanCFCFileWidget::on_pushButton_Scan_clicked()
{
    if(!m_SeleteFilesModel) return;

    QString path = ui->lineEdit->text();
    if(path.isEmpty()) return;

    QStringList &&list = ScanCFCFile::scanCFCFile(m_fc, path);

    m_SeleteFilesModel->setStringList(list);
}


void ScanCFCFileWidget::on_pushButton_AddList_clicked()
{
    if(!m_SeleteFilesModel) return;
    QStringList list = m_SeleteFilesModel->stringList();

    QStringList uploadFiles;
    QStringList downloadFiles;

    for(const auto &var : list) {
        if(var.contains(".CFCUpload_info")) {
            uploadFiles << var;
        } else if(var.contains(".CFCDownload_info")) {
            downloadFiles << var;
        }
    }

    this->hide();

    if(!uploadFiles.isEmpty()) {
        emit sigScanUpFiles(uploadFiles);
    }

    if(!downloadFiles.isEmpty()) {
        emit sigScanDownFiles(downloadFiles);
    }


}


void ScanCFCFileWidget::on_pushButton_Del_clicked()
{
    if(!m_SeleteFilesModel) return;
    QModelIndexList selIndexs = ui->listView->selectionModel()->selectedRows();
    foreach (const QModelIndex& index, selIndexs) {
        m_SeleteFilesModel->removeRow(index.row());
    }
}


void ScanCFCFileWidget::on_pushButton_Close_clicked()
{
    this->close();
}

