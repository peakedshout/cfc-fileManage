#include "DownloadFileItem.h"
#include "ui_DownloadFileItem.h"

DownloadFileItem::DownloadFileItem(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DownloadFileItem)
{
    ui->setupUi(this);
    m_BtnState = true;
    ui->progressBar->setMinimum(0);
}

DownloadFileItem::~DownloadFileItem()
{
    delete ui;
}

void DownloadFileItem::slotDownloadProgress(ProgressContext progressInfo, bool finished)
{
    if(!m_BtnState) return;

    if(finished) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
        ui->pushButton_switch->setEnabled(false);
        ui->pushButton_switch->setText("已完成");
        progressInfo.status = "DownStatusDownloadCompleted";
        ui->label_speed->setText("0.00B/s");

    } else {
        if("UpStatusUploadFailure" == progressInfo.status) {
            ui->label_speed->setText("0.00B/s");
            slotBtnState(false);
        } else {
            ui->progressBar->setMaximum(progressInfo.total);
            ui->progressBar->setValue(progressInfo.currProgress);

            ui->label_speed->setText(progressInfo.downloadSpeedShow);

        }
    }

    QString str = "";
    if("DownStatusDownloadReset" == progressInfo.status) {
        str = "下载重置";
    } else if("DownStatusDownloadInProgress" == progressInfo.status) {
        str = "下载进行中";
    } else if("DownStatusDownloadCompleted" == progressInfo.status) {
        str = "下载完成";
    } else if("DownStatusDownloadFailure" == progressInfo.status) {
        str = "下载失败";
    } else if("DownStatusDownloadPause" == progressInfo.status) {
        str = "下载暂停";
    } else if("DownStatusDownloadFileNil" == progressInfo.status) {
        str = "下载文件不存在";
    }

    ui->label_state->setText(str);
    ui->label_name->setText(progressInfo.to);
    ui->label_name->setToolTip(progressInfo.to);
    ui->label_fileFrom->setText(progressInfo.from);
    ui->label_fileFrom->setToolTip(progressInfo.from);

    ui->progressBar->update();
}

void DownloadFileItem::slotBtnState(bool running)
{
    m_BtnState = running;
    if(!m_BtnState) {
        ui->pushButton_switch->setText("开始");
        ui->label_state->setText("下载暂停");
        ui->label_speed->setText("0.00B/s");
    } else {
        ui->pushButton_switch->setText("暂停");
        ui->label_state->setText("下载进行中");
    }
    emit sigDownloadState(ui->label_name->text(), m_BtnState);
}

void DownloadFileItem::slotFileFrom(const QString &from)
{
    ui->label_fileFrom->setText(from);
}

void DownloadFileItem::slotFileTo(const QString &to)
{
    ui->label_name->setText(to);
}

void DownloadFileItem::on_pushButton_switch_clicked()
{
    slotBtnState(!m_BtnState);
}

void DownloadFileItem::on_pushButton_cancel_clicked()
{
    if(ui->pushButton_switch->text() != "已完成") {
        int btn = QMessageBox::information(this, "下载任务", "任务未完成，是否取消", QMessageBox::Ok | QMessageBox::No, QMessageBox::No);
        if(btn != QMessageBox::Ok) {
            return;
        }
    }

    ui->label_speed->setText("0.00B/s");
    emit sigCancel(ui->label_name->text());
}


void DownloadFileItem::on_pushButton_ReDownload_clicked()
{
    m_BtnState = false;
    slotBtnState(m_BtnState);

    ui->label_speed->setText("0.00B/s");
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);

    ui->label_speed->setText(tr("下载重置"));
    ui->pushButton_switch->setEnabled(true);
    emit sigReset(ui->label_fileFrom->text(), ui->label_name->text());


}

