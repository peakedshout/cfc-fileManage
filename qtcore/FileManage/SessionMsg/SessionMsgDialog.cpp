#include "SessionMsgDialog.h"
#include "ui_SessionMsgDialog.h"

SessionMsgDialog::SessionMsgDialog(const QString &serverName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionMsgDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("信息");

    this->resize(750, 450);


    m_PieSliceSum = nullptr;
    m_PieSlice = nullptr;

    m_ServerName = serverName;

    setSessionMsg(m_ServerName);

    createpieSewies();
}

SessionMsgDialog::~SessionMsgDialog()
{
    delete ui;
}

void SessionMsgDialog::setSessionMsg(const QString &serverName)
{
    if(!giv_SessionMsg.sessions.contains(serverName)) return;

    SessionMsg::SessionInfo info = giv_SessionMsg.sessions.value(serverName);

    ui->lineEdit_ServerName->setText(info.serverName);
    ui->lineEdit_Ip->setText(info.ip);
    ui->lineEdit_Port->setText(QString::number(info.port));


    ui->spinBox_SessionNum->setValue(info.sessions);

    ui->spinBox_DownloadNum->setValue(info.downloadNum);
    ui->spinBox_UploadNum->setValue(info.uploadNum);

    ui->lineEdit_LocalPath->setText(info.localPath);
    ui->lineEdit_ScanPath->setText(info.scanPath);
    ui->lineEdit_DownloadPath->setText(info.savePath);
}

void SessionMsgDialog::slotMemoryCapacity(quint64 value, quint64 min, quint64 max, const QString &valueText, const QString &maxText)
{
    m_PieSliceSum->setValue(max-min);
    m_PieSliceSum->setLabel("容量:" + maxText);

    m_PieSlice->setValue(value);
    m_PieSlice->setLabel("已占容量:" + valueText);
}

void SessionMsgDialog::on_pushButton_Close_clicked()
{
    this->close();
}


void SessionMsgDialog::on_pushButton_Quit_clicked()
{
    int btn = QMessageBox::question(this, "退出", "是否退出");
    if(btn == QMessageBox::Yes) {
        accept();
        emit sigQuit(m_ServerName);
    }

}


void SessionMsgDialog::on_pushButton_Create_clicked()
{
    accept();
    emit sigCreate();
}

//创建饼状图
void SessionMsgDialog::createpieSewies()
{
    //饼状图
    QPieSeries *pieSeries = new QPieSeries(this);
    //中间圆与大圆的比例
    pieSeries->setHoleSize(0.45);

    m_PieSlice = new QPieSlice(this);
    m_PieSlice->setValue(25);
    m_PieSlice->setLabel("已占容量");
    QString color = "#53b666";
    m_PieSlice->setColor(QColor());
    m_PieSlice->setLabelColor(QColor(color));
    m_PieSlice->setBorderColor(QColor(color));
    m_PieSlice->setLabelVisible();//设置标签可见,缺省不可见
    pieSeries->append(m_PieSlice);


    //扇形及数据
    m_PieSliceSum = new QPieSlice(this);
    m_PieSliceSum->setValue(25);//扇形占整个圆的百分比
    m_PieSliceSum->setLabel("总容量");
    m_PieSliceSum->setLabelVisible();
    color = "#4cb9cf";
    m_PieSliceSum->setColor(QColor(color));
    m_PieSliceSum->setLabelColor(QColor(color));
    m_PieSliceSum->setBorderColor(QColor(color));
    pieSeries->append(m_PieSliceSum);


    // 图表视图
    QChart *chart = new QChart();
    chart->setTitle("容量占表");
    chart->addSeries(pieSeries);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setBackgroundVisible(false);
//    chart->legend()->setFont(QFont("黑体", 8)) ; // 图例字体
    chart->setTitleBrush(QColor("#808396"));
    chart->legend()->setLabelColor(QColor("#808396"));

    QChartView *chartView = new QChartView(ui->widget);
    chartView->setRenderHint(QPainter::Antialiasing);
//    chartView->setRenderHint(QPainter::NonCosmeticDefaultPen);
    chartView->setChart(chart);
    ui->gridLayout->addWidget(chartView);
}


void SessionMsgDialog::on_spinBox_SessionNum_valueChanged(int arg1)
{
    giv_SessionMsg.sessions[m_ServerName].sessions = arg1;
}


void SessionMsgDialog::on_spinBox_UploadNum_valueChanged(int arg1)
{
    giv_SessionMsg.sessions[m_ServerName].uploadNum = arg1;
}


void SessionMsgDialog::on_spinBox_DownloadNum_valueChanged(int arg1)
{
    giv_SessionMsg.sessions[m_ServerName].downloadNum = arg1;
}

