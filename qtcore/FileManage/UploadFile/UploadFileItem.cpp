#include "UploadFileItem.h"
#include "ui_UploadFileItem.h"

UploadFileItem::UploadFileItem(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::UploadFileItem)
{
    ui->setupUi(this);
    m_BtnState = true;
    ui->progressBar->setMinimum(0);
}

UploadFileItem::~UploadFileItem()
{
    delete ui;
}

void UploadFileItem::slotDownloadProgress(ProgressContext progressInfo, bool finished)
{
    if(finished) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
        ui->pushButton_switch->setEnabled(false);
        ui->pushButton_switch->setText("已完成");
        progressInfo.status = "UpStatusUploadCompleted";
        ui->label_speed->setText("0.00B/s");
    } else {
        if("UpStatusUploadFailure" == progressInfo.status) {
            ui->label_speed->setText("0.00B/s");
            slotBtnState(false);
        } else {
            ui->progressBar->setMaximum(progressInfo.total);
            ui->progressBar->setValue(progressInfo.currProgress);

            ui->label_speed->setText(progressInfo.uploadSpeedShow);

        }
    }

    QString str = "";
    if("UpStatusUploadReset" == progressInfo.status) {
        str = "上传重置";
    } else if("UpStatusUploadInProgress" == progressInfo.status) {
        str = "上传进行中";
    } else if("UpStatusUploadCompleted" == progressInfo.status) {
        str = "上传完成";
    } else if("UpStatusUploadFailure" == progressInfo.status) {
        str = "上传失败";
    } else if("UpStatusUploadPause" == progressInfo.status) {
        str = "上传暂停";
    } else if("UpStatusUploadFileNil" == progressInfo.status) {
        str = "上传文件不存在";
    }

    ui->label_state->setText(str);
    ui->label_name->setText(progressInfo.to);
    ui->label_name->setToolTip(progressInfo.to);
    ui->label_fileFrom->setText(progressInfo.from);
    ui->label_fileFrom->setToolTip(progressInfo.from);

    ui->progressBar->update();
}

void UploadFileItem::slotBtnState(bool running)
{
    m_BtnState = running;
    if(!m_BtnState) {
        ui->pushButton_switch->setText("开始");
        ui->label_state->setText("上传暂停");
        ui->label_speed->setText("0.00B/s");
    } else {
        ui->pushButton_switch->setText("暂停");
        ui->label_state->setText("上传进行中");
    }
    emit sigUploadState(ui->label_fileFrom->text(), m_BtnState);
}

void UploadFileItem::on_pushButton_switch_clicked()
{
    slotBtnState(!m_BtnState);
}

void UploadFileItem::on_pushButton_cancel_clicked()
{
    ui->label_speed->setText("0.00B/s");
    emit sigCancel(ui->label_fileFrom->text());
}

