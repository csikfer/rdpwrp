#include "idletimeout.h"
#include "ui_idletimeout.h"
#include <QPushButton>
#include "dialog.h"

cIdleTimeOut::cIdleTimeOut(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::idleTimeOut)
{
    ui->setupUi(this);
    setStyleSheet("background-color: yellow");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(trUtf8("Ne!"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(trUtf8("Kikapcsolás!"));
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
    if (idleTime >= IDLEDIALOGTIME) accept();
    int i = IDLEDIALOGTIME - idleTime;
    setStyleSheet(QString("background-color: rgb(255,%1,0)").arg((255 * i) / IDLEDIALOGTIME));
    ui->lcdNumber->display(i);
}

