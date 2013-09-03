#ifndef IDLETIMEOUT_H
#define IDLETIMEOUT_H

#include <QDialog>
#include "ui_idletimeout.h"

namespace Ui {
class idleTimeOut;
}

class cIdleTimeOut : public QDialog
{
    Q_OBJECT
public:
    explicit cIdleTimeOut(bool _kiosk, QWidget *parent = NULL);
    ~cIdleTimeOut();

protected:
    void timerEvent(QTimerEvent *);
    int         waitTime;
    int         timeCnt;
    int         timerId;
private:
    Ui::idleTimeOut *ui;
};

#endif // IDLETIMEOUT_H
