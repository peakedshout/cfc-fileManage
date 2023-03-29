#include "ConnectInfoDialog.h"
#include "ui_ConnectInfoDialog.h"
#include "System/SystemInfo.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/CFCFileApiParse.h"
#include "FileManageApi/RemoteFileManageApi.h"

ConnectInfoDialog::ConnectInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectInfoDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setWindowTitle("连接信息");

    ui->pushButton_connect->setFocus();

    giv_SessionMsg.readSessionMsg();

    QList values = giv_SessionMsg.sessions.values();
    foreach (const auto &var, values) {
        ui->comboBox->addItem(var.serverName);
    }

    m_ServerName = ui->comboBox->currentText();

    sessionMsg();

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [=](const QString &text) {
        m_ServerName = text;
        sessionMsg();
    });
}

ConnectInfoDialog::~ConnectInfoDialog()
{
//    giv_SessionMsg.wirteSessionMsg();

    QList values = m_RegisterMap.values();
    for(const auto &var : values) {
        Close(var);
    }

    delete ui;
}

void ConnectInfoDialog::slotSessionQuit(const QString &ipAndPort)
{
    m_LoginList.removeOne(ipAndPort);
}

void ConnectInfoDialog::sessionMsg()
{
    ui->lineEdit_serverName->setText(giv_SessionMsg.sessions.value(m_ServerName).serverName);

    ui->lineEdit_ip->setText(giv_SessionMsg.sessions.value(m_ServerName).ip);

    ui->lineEdit_port->setText(QString::number(giv_SessionMsg.sessions.value(m_ServerName).port));

    ui->lineEdit_key->setText(giv_SessionMsg.sessions.value(m_ServerName).key);

    ui->lineEdit_fileKey->setText(giv_SessionMsg.sessions.value(m_ServerName).fileKey);

    ui->lineEdit_localDir->setText(giv_SessionMsg.sessions.value(m_ServerName).localPath);

    ui->lineEdit_ScanDir->setText(giv_SessionMsg.sessions.value(m_ServerName).scanPath);

    ui->lineEdit_SaveDir->setText(giv_SessionMsg.sessions.value(m_ServerName).savePath);

}

void ConnectInfoDialog::on_pushButton_selectDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,"请选择一个目录", ui->lineEdit_localDir->text());
    if(dir.isEmpty() || ui->lineEdit_localDir->text() == dir) return;
    ui->lineEdit_localDir->setText(dir);
}

void ConnectInfoDialog::on_pushButton_connect_clicked()
{
    if(ui->lineEdit_serverName->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查设备名称...");
        return;
    }
    if(ui->lineEdit_ip->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查设备Ip...");
        return;
    }
    if(ui->lineEdit_port->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查端口...");
        return;
    }
    if(ui->lineEdit_key->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查信道密钥...");
        return;
    }
    if(ui->lineEdit_fileKey->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查文件密钥...");
        return;
    }
    if(ui->lineEdit_localDir->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查本地目录...");
        return;
    }
    if(ui->lineEdit_ScanDir->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查扫描目录...");
        return;
    }
    if(ui->lineEdit_SaveDir->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查下载目录...");
        return;
    }

    if(m_LoginList.count() >= giv_SessionMsg.sessions.value(ui->lineEdit_serverName->text()).sessions) {
        QMessageBox::information(this, "创建会话", "超过创建会话最大数值");
        return;
    }

    if(giv_SessionMsg.prove != SystemInfo::getBaseboardUuid()) {
        QMessageBox::critical(this, "验证", "验证不通过，配置文件不符，请卸载config.ini文件");
        return;
    }

    if(giv_SessionMsg.prove.isEmpty()) {
        int btn = QMessageBox::information(this, "验证", "验证信息为空，是否登录", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(btn == QMessageBox::No) return;
    }

    SessionMsg::SessionInfo sessionInfo = giv_SessionMsg.sessions.value(ui->lineEdit_serverName->text());
    sessionInfo.serverName = ui->lineEdit_serverName->text();
    sessionInfo.ip = ui->lineEdit_ip->text();
    sessionInfo.port = ui->lineEdit_port->text().toInt();
    sessionInfo.key = ui->lineEdit_key->text();
    sessionInfo.fileKey = ui->lineEdit_fileKey->text();
    sessionInfo.localPath = ui->lineEdit_localDir->text();
    sessionInfo.scanPath = ui->lineEdit_ScanDir->text();
    sessionInfo.savePath = ui->lineEdit_SaveDir->text();

    giv_SessionMsg.sessions.insert(ui->lineEdit_serverName->text(), sessionInfo);


    QString loginMapKey = ui->lineEdit_ip->text() + "+" + ui->lineEdit_port->text();

    if(m_LoginList.contains(loginMapKey)) {
        QMessageBox::information(this, "登录", "已登陆");
        return;
    }


    bool exist = m_RegisterMap.contains(loginMapKey);
    int c = -1;

    if(exist) {
        QMessageBox msgBox;
        bool rejected = false;
        connect(&msgBox, &QMessageBox::rejected, this, [&rejected](){
            rejected = true;
        });
        msgBox.setWindowTitle("客户端");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("是否复用客户端");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int btn = msgBox.exec();

        if(btn == QMessageBox::No) {
            exist = false;
        } else if(btn == QMessageBox::Cancel) {
            return;
        }

        if(rejected) return;

    }

    if(!exist) {
        GoStr go_name(giv_SessionMsg.prove.toUtf8());
        GoStr go_ip(QString(ui->lineEdit_ip->text()+ ":" + ui->lineEdit_port->text()).toUtf8());
//        GoStr go_port(':' + ui->lineEdit_port->text().toUtf8());
        GoStr go_key(ui->lineEdit_key->text().toUtf8());
        QSharedPointer<char> initCFCMsg(InitCFC(go_name.getGoString(), go_ip.getGoString(), go_key.getGoString()));

        QString err = "";
        bool ok = CFCFileApiParse::initCFCInfo(initCFCMsg.data(), c, err);
        if(!ok || !err.isEmpty()) {
            QMessageBox::critical(this, "创建客户端", err);
            return;
        }

        if(c <= 0) {
            QMessageBox::critical(this, "创建客户端", QString("c的值为 %1").arg(c));
            return;
        }

        // 储存登录
        m_RegisterMap.insert(loginMapKey, c);
    } else {
        c = m_RegisterMap.value(loginMapKey);
    }



    if(c <= 0) {
        QMessageBox::information(this, "登录", QString("登录失败，c = %1").arg(c));
        return;
    }

    if(!m_LoginList.contains(loginMapKey)) {
        m_LoginList.append(loginMapKey);
    }

    GoStr go_serverName(ui->lineEdit_serverName->text().toUtf8());
    GoStr go_fileKey(ui->lineEdit_fileKey->text().toUtf8());
    QSharedPointer<char> initMsg(InitRemoteFileContext(c, go_serverName.getGoString(), go_fileKey.getGoString()));
    QByteArray msg(initMsg.data());

//    debugMsg(msg);
    int fc = -1;
    QString currPath;
    RemoteFileManageApi remoteFileManageApi;
    remoteFileManageApi.readerDir(msg, nullptr, fc, currPath);

    if(fc <= 0) {
        QMessageBox::information(this, "登录", QString("登录失败， fc = %1").arg(fc));
        return;
    }

    giv_SessionMsg.wirteSessionMsg();

    emit sigLoginMsg(fc, currPath, ui->lineEdit_serverName->text(), loginMapKey);
}


void ConnectInfoDialog::on_pushButton_selectDir_2_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,"请选择一个目录", ui->lineEdit_ScanDir->text());
    if(dir.isEmpty() || ui->lineEdit_ScanDir->text() == dir) return;
    ui->lineEdit_ScanDir->setText(dir);
}


void ConnectInfoDialog::on_pushButton_selectDir_3_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,"请选择一个目录", ui->lineEdit_SaveDir->text());
    if(dir.isEmpty() || ui->lineEdit_SaveDir->text() == dir) return;
    ui->lineEdit_SaveDir->setText(dir);
}


void ConnectInfoDialog::on_pushButton_Del_clicked()
{
    giv_SessionMsg.sessions.remove(ui->comboBox->currentText());
    ui->comboBox->removeItem(ui->comboBox->currentIndex());

    giv_SessionMsg.wirteSessionMsg();
}

