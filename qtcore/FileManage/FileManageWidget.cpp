#include "FileManageWidget.h"

FileManageWidget::FileManageWidget(int fc, const QString &clientName, const QString &currPath, const SessionInfo &info, QWidget *parent) : QWidget(parent)
{
    m_LocalWin = nullptr;
    m_RemoteWin = nullptr;

    QHBoxLayout *HBoxLayout = new QHBoxLayout(this);

    m_LocalWin = new LocalFileManageMainWin(clientName);
    m_RemoteWin = new RemoteFileManageMainWin(fc, clientName, currPath);

    m_fc = fc;
    m_pSessionInfo = info;

    HBoxLayout->addWidget(m_LocalWin);
    HBoxLayout->addWidget(m_RemoteWin);


    connect(m_LocalWin, &LocalFileManageMainWin::sigUpload, m_RemoteWin, &RemoteFileManageMainWin::slotUpload);
    connect(m_LocalWin, &LocalFileManageMainWin::sigAddDownloadFiles, m_RemoteWin, &RemoteFileManageMainWin::slotAddDownloadFiles);
    connect(m_LocalWin, &LocalFileManageMainWin::sigAddUpFiles, m_RemoteWin, &RemoteFileManageMainWin::slotAddUploadFiles);

    connect(m_RemoteWin, &RemoteFileManageMainWin::sigCreate, this, &FileManageWidget::sigCreate);
    connect(m_RemoteWin, &RemoteFileManageMainWin::sigQuit, this, &FileManageWidget::sigQuit);
    connect(m_RemoteWin, &RemoteFileManageMainWin::sigReConnection, this, [this](bool isExit_C){
        emit sigReConnection(m_pSessionInfo, isExit_C);
    });

}

FileManageWidget::~FileManageWidget()
{
    if(m_LocalWin) m_LocalWin->deleteLater();
    if(m_RemoteWin) m_RemoteWin->deleteLater();
    emit sigCloseFc(m_fc);
}

void FileManageWidget::slotReconSucceed(const QString &clientName, const SessionInfo &pSessionInfo, int fc)
{
    m_fc = fc;
    m_pSessionInfo = pSessionInfo;
    if(m_RemoteWin) m_RemoteWin->setfc(fc);
}
