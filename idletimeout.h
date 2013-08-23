#ifndef IDLETIMEOUT_H
#define IDLETIMEOUT_H

#include <QDialog>

namespace Ui {
class idleTimeOut;
}

class cIdleTimeOut : public QDialog
{
    Q_OBJECT
    
public:
    explicit cIdleTimeOut(QWidget *parent = 0);
    ~cIdleTimeOut();

protected:
    void timerEvent(QTimerEvent *);
    int         downCnt;
    int         timerId;
private:
    Ui::idleTimeOut *ui;
};

#endif // IDLETIMEOUT_H
