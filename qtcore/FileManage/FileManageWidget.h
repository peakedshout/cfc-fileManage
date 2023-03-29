#ifndef FILEMANAGEWIDGET_H
#define FILEMANAGEWIDGET_H

#include <QWidget>
#include "FileManageWidget/LocalFileManageMainWin.h"
#include "FileManageWidget/RemoteFileManageMainWin.h"
#include <QHBoxLayout>

class FileManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileManageWidget(int fc, const QString &serverName, const QString &currPath, QWidget *parent = nullptr);
    ~FileManageWidget();

signals:
    void sigCreate();
    void sigQuit(const QString &name);

private:
    LocalFileManageMainWin *m_LocalWin;
    RemoteFileManageMainWin *m_RemoteWin;
};

#endif // FILEMANAGEWIDGET_H
