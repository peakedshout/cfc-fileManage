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

    ui->toolButton_2->setIcon(QIcon(":/png/yanjing_xianshi_o.png"));
    ui->toolButton->setIcon(QIcon(":/png/yanjing_xianshi_o.png"));

    giv_SessionMsg.readSessionMsg();

    QList values = giv_SessionMsg.sessions.values();
    foreach (const auto &var, values) {
        ui->comboBox->addItem(var.serverName + "-" + var.clientName);
    }

    m_ClientName = ui->comboBox->currentText();
//    QStringList &&list = ui->comboBox->currentText().split('-');
//    if(list.size() > 1) {
//        m_ClientName = list.at(1);
//    } else {
//        QMessageBox::critical(this, "错误", "代理名称-用户名称的格式错误");
//    }

    sessionMsg();

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [=](const QString &text) {
//        QStringList &&list = text.split('-');
//        if(list.size() > 1) {
//            m_ClientName = list.at(1);
//        } else {
//            QMessageBox::critical(this, "错误", "代理名称-用户名称的格式错误");
//        }
        m_ClientName = text;
        sessionMsg();
    });

}

ConnectInfoDialog::~ConnectInfoDialog()
{
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

void ConnectInfoDialog::slotReConnection(const SessionInfo &pSessionInfo, bool isExit_C)
{
    int c = -1;
    QString n = pSessionInfo.dlName;
    if(!isExit_C) {
        n = giv_SessionMsg.prove + "_" + QString::number(num);
        GoStr go_name(n.toUtf8());

        GoStr go_ip(pSessionInfo.ip.toUtf8());
        GoStr go_key(pSessionInfo.key.toUtf8());
        QSharedPointer<char> initCFCMsg(InitCFC(go_name.getGoString(), go_ip.getGoString(), go_key.getGoString()));

        QString err = "";
        bool ok = CFCFileApiParse::initCFCInfo(initCFCMsg.data(), c, err);
        if(!ok || !err.isEmpty()) {
//            QMessageBox::critical(this, "创建客户端", "重连失败");
            return;
        }

        QList<int> l;
        m_CManage.C_FCsMap.insert(c, l);
    } else {
        m_CManage.findC(pSessionInfo.ip, c);
    }

    if(c <= 0) {
        return;
    }

    GoStr go_serverName(pSessionInfo.serName.toUtf8());
    GoStr go_clientName(pSessionInfo.cliName.toUtf8());
    GoStr go_fileKey(pSessionInfo.filekey.toUtf8());
    QSharedPointer<char> initMsg(InitRemoteFileContext(c, go_serverName.getGoString(), go_clientName.getGoString(), go_fileKey.getGoString()));
    QByteArray msg(initMsg.data());


    int fc = -1;
    QString currPath;
    RemoteFileManageApi remoteFileManageApi;
    remoteFileManageApi.readerDir(msg, nullptr, fc, currPath);
    if(fc <= 0) {
//        slotClose(pSessionInfo.fc);
//        QMessageBox::critical(this, "创建客户端", "重连失败");
        return;
    }

    QList<int> fcList = m_CManage.C_FCsMap.value(c);
    if(!fcList.contains(fc)) {
        fcList.append(fc);
    }
    m_CManage.C_FCsMap.insert(c, fcList);

    SessionInfo liv_sessionInfo;
    liv_sessionInfo.c = c;
    liv_sessionInfo.fc = fc;
    liv_sessionInfo.ip = pSessionInfo.ip;
    liv_sessionInfo.dlName = n;
    liv_sessionInfo.cliName = pSessionInfo.cliName;
    liv_sessionInfo.serName = pSessionInfo.serName;
    liv_sessionInfo.filekey = pSessionInfo.filekey;
    liv_sessionInfo.key = pSessionInfo.key;
    ++num;
    m_CManage.fcInfo.insert(fc, liv_sessionInfo);
    slotClose(pSessionInfo.fc);
    emit sigReconSucceed(pSessionInfo.cliName, liv_sessionInfo, fc);
}

void ConnectInfoDialog::slotClose(int fc)
{
    m_CManage.removeFc(fc);
    m_CManage.closeCBy0();
}

void ConnectInfoDialog::sessionMsg()
{
    ui->lineEdit_clientName->setText(giv_SessionMsg.sessions.value(m_ClientName).clientName);

    ui->lineEdit_serverName->setText(giv_SessionMsg.sessions.value(m_ClientName).serverName);

    ui->lineEdit_ip->setText(giv_SessionMsg.sessions.value(m_ClientName).ip);

    ui->lineEdit_key->setText(giv_SessionMsg.sessions.value(m_ClientName).key);

    ui->lineEdit_fileKey->setText(giv_SessionMsg.sessions.value(m_ClientName).userKey);

    ui->lineEdit_localDir->setText(giv_SessionMsg.sessions.value(m_ClientName).localPath);

    ui->lineEdit_ScanDir->setText(giv_SessionMsg.sessions.value(m_ClientName).scanPath);

    ui->lineEdit_SaveDir->setText(giv_SessionMsg.sessions.value(m_ClientName).savePath);

}

void ConnectInfoDialog::on_pushButton_selectDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,"请选择一个目录", ui->lineEdit_localDir->text());
    if(dir.isEmpty() || ui->lineEdit_localDir->text() == dir) return;
    ui->lineEdit_localDir->setText(dir);
}

void ConnectInfoDialog::on_pushButton_connect_clicked()
{
    if(ui->lineEdit_clientName->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查用户名称...");
        return;
    }

    if(ui->lineEdit_serverName->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查设备名称...");
        return;
    }

    if(ui->lineEdit_ip->text().isEmpty()) {
        QMessageBox::information(this, "连接错误！", "请检查设备Ip...");
        return;
    }

    QRegExp regExpIp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])"
                  ":([0-9]|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{4}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])");
    if (!regExpIp.exactMatch(ui->lineEdit_ip->text())) {
        QMessageBox::information(this, "提示", "IP与端口格式错误");
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

    if(m_LoginList.count() >= giv_SessionMsg.sessions.value(ui->lineEdit_clientName->text()).sessions) {
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

    SessionMsg::SessionInfo sessionInfo = giv_SessionMsg.sessions.value(ui->lineEdit_clientName->text());
    sessionInfo.clientName = ui->lineEdit_clientName->text();
    sessionInfo.serverName = ui->lineEdit_serverName->text();
    sessionInfo.ip = ui->lineEdit_ip->text();
    sessionInfo.key = ui->lineEdit_key->text();
    sessionInfo.userKey = ui->lineEdit_fileKey->text();
    sessionInfo.localPath = ui->lineEdit_localDir->text();
    sessionInfo.scanPath = ui->lineEdit_ScanDir->text();
    sessionInfo.savePath = ui->lineEdit_SaveDir->text();

    giv_SessionMsg.sessions.insert(ui->lineEdit_clientName->text(), sessionInfo);


    QString loginMapKey = ui->lineEdit_ip->text() + "_" + ui->lineEdit_clientName->text() + "_" + ui->lineEdit_serverName->text();

    if(m_LoginList.contains(loginMapKey)) {
        QMessageBox::information(this, "登录", "已登陆");
        return;
    }


    int c = -1;
    bool exist = m_CManage.findC(ui->lineEdit_ip->text(), c);
    QString n;
    if(exist) {
        n = m_CManage.C_DLMap.value(c);
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
            rejected = false;
        } else if(btn == QMessageBox::Cancel) {
            return;
        }

        if(rejected) return;

    }

    if(!exist) {
        n = giv_SessionMsg.prove;
        if(num != 0) {
            n += QString("_%1").arg(num);
        }
        GoStr go_name(n.toUtf8());
        GoStr go_ip(ui->lineEdit_ip->text().toUtf8());
        GoStr go_key(ui->lineEdit_key->text().toUtf8());
        QSharedPointer<char> initCFCMsg(InitCFC(go_name.getGoString(), go_ip.getGoString(), go_key.getGoString()));

        QString err = "";
        bool ok = CFCFileApiParse::initCFCInfo(initCFCMsg.data(), c, err);
        if(!ok || !err.isEmpty()) {
            QMessageBox::critical(this, "创建客户端", err);
            return;
        }

        if(c <= 0) {
            QMessageBox::critical(this, "创建客户端", "创建客户端失败");
            return;
        }

        // 储存登录
        QList<int> l;
        m_CManage.C_FCsMap.insert(c, l);
        ++num;
    }

    if(c <= 0) {
        QMessageBox::information(this, "登录", QString("登录失败").arg(c));
        return;
    }

    m_CManage.C_DLMap.insert(c, n);
    GoStr go_serverName(ui->lineEdit_serverName->text().toUtf8());
    GoStr go_clientName(ui->lineEdit_clientName->text().toUtf8());
    GoStr go_fileKey(ui->lineEdit_fileKey->text().toUtf8());
    QSharedPointer<char> initMsg(InitRemoteFileContext(c, go_serverName.getGoString(), go_clientName.getGoString(), go_fileKey.getGoString()));
    QByteArray msg(initMsg.data());

    int fc = -1;
    QString currPath;
    RemoteFileManageApi remoteFileManageApi;
    remoteFileManageApi.readerDir(msg, nullptr, fc, currPath);

    if(fc <= 0) {
        m_CManage.closeCBy0();
        QMessageBox::information(this, "登录", "登录失败");
        return;
    }

    QList<int> fcList = m_CManage.C_FCsMap.value(c);
    if(!fcList.contains(fc)) {
        fcList.append(fc);
    }
    m_CManage.C_FCsMap.insert(c, fcList);

    SessionInfo liv_sessionInfo;
    liv_sessionInfo.c = c;
    liv_sessionInfo.fc = fc;
    liv_sessionInfo.ip = ui->lineEdit_ip->text();
    liv_sessionInfo.dlName = n;
    liv_sessionInfo.cliName = ui->lineEdit_clientName->text();
    liv_sessionInfo.serName = ui->lineEdit_serverName->text();
    liv_sessionInfo.filekey = ui->lineEdit_fileKey->text();
    liv_sessionInfo.key = ui->lineEdit_key->text();

    m_CManage.fcInfo.insert(fc, liv_sessionInfo);

    if(!m_LoginList.contains(loginMapKey)) {
        m_LoginList.append(loginMapKey);
    }

    if(ui->checkBox->isChecked()) {
        giv_SessionMsg.save();
        giv_SessionMsg.wirteSessionMsg();
    }

    emit sigLoginMsg(fc, currPath, ui->lineEdit_clientName->text(), loginMapKey, liv_sessionInfo);
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
    giv_SessionMsg.save();
    giv_SessionMsg.wirteSessionMsg();
    ui->comboBox->removeItem(ui->comboBox->currentIndex());
}


void ConnectInfoDialog::on_toolButton_2_toggled(bool checked)
{
    if(checked) {
        ui->toolButton_2->setIcon(QIcon(":/png/yanjing_xianshi_o.png"));
        ui->lineEdit_key->setEchoMode(QLineEdit::Password);
    } else {
        ui->toolButton_2->setIcon(QIcon(":/png/yanjing_yincang_o.png"));
        ui->lineEdit_key->setEchoMode(QLineEdit::Normal);
    }

}

void ConnectInfoDialog::on_toolButton_toggled(bool checked)
{
    if(checked) {
        ui->toolButton->setIcon(QIcon(":/png/yanjing_xianshi_o.png"));
        ui->lineEdit_fileKey->setEchoMode(QLineEdit::Password);
    } else {
        ui->toolButton->setIcon(QIcon(":/png/yanjing_yincang_o.png"));
        ui->lineEdit_fileKey->setEchoMode(QLineEdit::Normal);
    }
}

