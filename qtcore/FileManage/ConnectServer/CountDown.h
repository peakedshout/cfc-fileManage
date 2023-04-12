#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QWidget>

namespace Ui {
class CountDown;
}

class CountDown : public QWidget
{
    Q_OBJECT

public:
    explicit CountDown(QWidget *parent = nullptr);
    ~CountDown();

    void slotLCDNum(int num);

signals:
    void sigNoReConn();
    void sigReConnection();

private:
    Ui::CountDown *ui;
};

#endif // COUNTDOWN_H
