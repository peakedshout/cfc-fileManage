#include "SessionMsgDialog.h"
#include "ui_SessionMsgDialog.h"
#include "RewriteApi/GoStr.h"


SessionMsgDialog::SessionMsgDialog(const QString &serverName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionMsgDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("用户信息");
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->resize(750, 450);


    m_PieSliceSum = nullptr;
    m_PieSlice = nullptr;
    m_OpenLog = false;
    m_Process = nullptr;
    m_DIp = -1;

    m_ServerName = serverName;

    setSessionMsg(m_ServerName);

    createpieSewies();

    connect(ui->radioButton_UseStack, &QRadioButton::toggled, this, &SessionMsgDialog::slotRadioButtonToggled);

    const QString &text = QString("<a style='color: #000; text-decoration: none'; href = %1>%2").arg(GetAddr()).arg(GetVersionInfo());
    ui->label_Version->setText(text);

    //连接信号槽
    connect(ui->label_Version, &QLabel::linkActivated, [=](QString url){
        QDesktopServices::openUrl(QUrl(url));
    });

    ui->pushButton_Close->setFocus();
}

SessionMsgDialog::~SessionMsgDialog()
{
    if(m_Process) {
        m_Process->close();
        m_Process->waitForFinished();

    }

    delete ui;
}

void SessionMsgDialog::setSessionMsg(const QString &serverName)
{
    if(!giv_SessionMsg.sessions.contains(serverName)) return;

    SessionMsg::SessionInfo info = giv_SessionMsg.sessions.value(serverName);

    ui->lineEdit_ServerName->setText(info.serverName);
    ui->lineEdit_Ip->setText(info.ip);
//    ui->lineEdit_Port->setText(QString::number(info.port));


    ui->comboBox_LogLevel->setCurrentIndex(info.logLevel);
    ui->radioButton_UseStack->setChecked(info.useStack);
    ui->spinBox_LogPort->setValue(info.logPort);


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

void SessionMsgDialog::on_pushButton_OpenLog_clicked()
{
    // 默认关闭
    m_OpenLog = !m_OpenLog;

    if(m_OpenLog) {
        ui->pushButton_OpenLog->setText("关闭");

        // 打开
        LogOpt(giv_SessionMsg.sessions.value(m_ServerName).logLevel, ui->radioButton_UseStack->isChecked());

        QSharedPointer<char> msg(LogListen(ui->spinBox_LogPort->value()));

        if(QString(msg.data()).trimmed().isEmpty()) {

            //打开进程
            if(!m_Process) {
                m_Process = new QProcess(this);
            }

            QString cmd = QString("%1 -p %2").arg(".\\cfc_fileManageApp_logdialer.exe").arg(ui->spinBox_LogPort->value());
            m_Process->setCreateProcessArgumentsModifier([] (QProcess::CreateProcessArguments *args)
            {
                args->flags |= CREATE_NEW_CONSOLE;
                args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
                args->startupInfo->dwFlags |= STARTF_USEFILLATTRIBUTE;
                args->startupInfo->dwFillAttribute = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
            });

            m_Process->start("powershell.exe", QStringList()<<"/c"<< cmd);
            m_Process->waitForStarted();

        }

    } else {
        ui->pushButton_OpenLog->setText("打开");

        if(m_Process) {
            m_Process->execute("taskkill /im cfc_fileManageApp_logdialer.exe /f");
            m_Process->close();
            m_Process->waitForFinished();
//            delete m_Process;
//            m_Process = nullptr;
        }
    }
}

void SessionMsgDialog::on_comboBox_LogLevel_currentIndexChanged(int index)
{
    giv_SessionMsg.sessions[m_ServerName].logLevel = index;
}

void SessionMsgDialog::slotRadioButtonToggled(bool checked)
{
    giv_SessionMsg.sessions[m_ServerName].useStack = checked;
}

void SessionMsgDialog::on_pushButton_Abort_clicked()
{
    QDialog dialog;
    dialog.setWindowTitle("关于");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    dialog.resize(500,450);

    QString res = GetAbout();

    QStringList &&list = res.split('\n');

//    ## 软件名：cfc-fileManage(client-qt)
//    ### 版本：cfc-fileManage-v0.1.0-beta.1
//    ###### 这是一个免费的文件管理工具，目前还是测试版本
//    ***
//    ### 项目地址：[cfc-fileManage](https://github.com/peakedshout/cfc-fileManage)
//    ### 作者列表：
//    - [peakedshout](https://github.com/peakedshout)(后端业务)
//    - [BUDAI-AZ](https://github.com/BUDAI-AZ)(qt页面)
//    ***
//    ##### 底层通信协议：[go-CFC](https://github.com/peakedshout/go-CFC)

    QString text;

    foreach (QString var, list) {
        var = var.trimmed();
        QString text1,link;
//        debugMsg(var);
        if(var.startsWith("###### ")) {
            var.remove("######");
            var = var.trimmed();

            if(link.isEmpty()) link = "#";

            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }

            text += QString("<h6><a href=\"%2\">%3</a>%1</h6>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("##### ")) {
            var.remove("#####");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }

            text += QString("<h5><a href=\"%2\">%3</a>%1</h5>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("#### ")) {
            var.remove("####");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }

            text += QString("<h4><a href=\"%2\">%3</a>%1</h4>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("### ")) {
            var.remove("###");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }
            text += QString("<h3><a href=\"%2\">%3</a>%1</h3>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("## ")) {
            var.remove("## ");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }
            text += QString("<h2><a href=\"%2\">%3</a>%1</h2>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("# ")) {
            var.remove("#");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }

            text += QString("<h1><a href=\"%2\">%3</a>%1</h1>\n").arg(var).arg(link).arg(text1);
        } else if(var.startsWith("***")) {
            text += QString("<hr size=5>\n");
        } else if(var.startsWith("- ")) {
            var.remove("- ");
            var = var.trimmed();
            if(link.isEmpty()) link = "#";
            const QString && t = parserMarkdown_Link(var, text1, link);
            if(!t.isEmpty()) {
                var = t;
            }
            text += QString("<ul><li><a href=\"%2\">%3</a>%1</li></ul>\n").arg(var).arg(link).arg(text1);
        }
    }



    QTextBrowser textBrowser;
    textBrowser.setOpenLinks(true);
    textBrowser.setOpenExternalLinks(true);
    textBrowser.setAcceptRichText(false);


    connect (&textBrowser, &QTextBrowser::sourceChanged,[&](QUrl u){
        QDesktopServices::openUrl(u);
    });

    textBrowser.setReadOnly(true);
    textBrowser.setHtml(text);

    QHBoxLayout layout(&dialog);
    layout.addWidget(&textBrowser);

    dialog.exec();
}

QString SessionMsgDialog::parserMarkdown_Link(QString msg, QString &text, QString &link)
{
    //        <a href="">关键词</a>
    if(!msg.contains("](")) {
        return "";
    }

//    [peakedshout](https://github.com/peakedshout)(后端业务)
//底层通信协议：[go-CFC](https://github.com/peakedshout/go-CFC)
    int index = msg.lastIndexOf("](");

    int i = msg.lastIndexOf('[', index);
    text = msg.mid(i+1, index-i-1);

    int ii = msg.indexOf(')', index);
    link = msg.mid(index+2, ii-index-2);

    msg.remove(i, ii-i+1);

    return msg;
}
