#include "CountDown.h"
#include "ui_CountDown.h"

CountDown::CountDown(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CountDown)
{
    ui->setupUi(this);

    connect(ui->pushButtonReConn, &QPushButton::clicked, this, &CountDown::sigReConnection);
    connect(ui->pushButton_fq, &QPushButton::clicked, this, &CountDown::sigNoReConn);
}

CountDown::~CountDown()
{
    delete ui;
}

void CountDown::slotLCDNum(int num)
{
    ui->lcdNumber->display(num);
}



