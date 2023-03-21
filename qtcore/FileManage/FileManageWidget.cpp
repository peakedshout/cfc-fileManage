#include "FileManageWidget.h"

FileManageWidget::FileManageWidget(int fc, const QString &serverName, const QString &currPath, QWidget *parent) : QWidget(parent)
{
    m_LocalWin = nullptr;
    m_RemoteWin = nullptr;

    QHBoxLayout *HBoxLayout = new QHBoxLayout(this);

    m_LocalWin = new LocalFileManageMainWin(serverName);
    m_RemoteWin = new RemoteFileManageMainWin(fc, serverName, currPath);



    HBoxLayout->addWidget(m_LocalWin);
    HBoxLayout->addWidget(m_RemoteWin);


    connect(m_LocalWin, &LocalFileManageMainWin::sigUpload, m_RemoteWin, &RemoteFileManageMainWin::slotUpload);
    connect(m_RemoteWin, &RemoteFileManageMainWin::sigCreate, this, &FileManageWidget::sigCreate);
    connect(m_RemoteWin, &RemoteFileManageMainWin::sigQuit, this, &FileManageWidget::sigQuit);

}

FileManageWidget::~FileManageWidget()
{
    if(m_LocalWin) m_LocalWin->deleteLater();
    if(m_RemoteWin) m_RemoteWin->deleteLater();
}
