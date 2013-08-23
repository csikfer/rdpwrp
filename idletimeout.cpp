#include "idletimeout.h"
#include "ui_idletimeout.h"
#include <QPushButton>
#include "dialog.h"

cIdleTimeOut::cIdleTimeOut(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::idleTimeOut)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(trUtf8("Ne!"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Kikapcsolás!"));
    timerId = startTimer(1000);
    downCnt = IDLEDIALOGTIME;
    ui->lcdNumber->display(downCnt);
    setModal(true);
}

cIdleTimeOut::~cIdleTimeOut()
{
    killTimer(timerId);
    delete ui;
}

void cIdleTimeOut::timerEvent(QTimerEvent *)
{
    ++idleTime;
    if (idleTime >= IDLEDIALOGTIME) reject();
    ui->lcdNumber->display(IDLEDIALOGTIME - idleTime);
}

