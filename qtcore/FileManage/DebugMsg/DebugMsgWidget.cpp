#include "DebugMsgWidget.h"
#include "ui_DebugMsgWidget.h"

DebugMsgWidget::DebugMsgWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugMsgWidget)
{
    ui->setupUi(this);
    m_Debug = false;
}

DebugMsgWidget::~DebugMsgWidget()
{
    delete ui;
}

void DebugMsgWidget::setTitle(const QString &title)
{
    ui->groupBox->setTitle(title);
}

void DebugMsgWidget::slotAppendDebugMsg(const QString &msg)
{
    if(m_Debug) ui->textEdit->append(msg);
}

void DebugMsgWidget::on_pushButton_Clear_clicked()
{
    ui->textEdit->clear();
}


void DebugMsgWidget::on_pushButton_Close_clicked()
{
    this->close();
}


void DebugMsgWidget::on_pushButton_StartAndPause_clicked()
{
    m_Debug = !m_Debug;

    if(m_Debug) {
        ui->pushButton_StartAndPause->setText("暂停");
    } else {
        ui->pushButton_StartAndPause->setText("开始");
    }

}

