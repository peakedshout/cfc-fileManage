#ifndef DEBUGMSGWIDGET_H
#define DEBUGMSGWIDGET_H

#include <QWidget>

namespace Ui {
class DebugMsgWidget;
}

class DebugMsgWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugMsgWidget(QWidget *parent = nullptr);
    ~DebugMsgWidget();

    void setTitle(const QString &title);

public slots:
    void slotAppendDebugMsg(const QString &msg);

private slots:
    void on_pushButton_Clear_clicked();

    void on_pushButton_Close_clicked();

    void on_pushButton_StartAndPause_clicked();

private:
    Ui::DebugMsgWidget *ui;

    bool m_Debug;
};

#endif // DEBUGMSGWIDGET_H
