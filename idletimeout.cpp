#include "idletimeout.h"
#include <QPushButton>
#include "dialog.h"

cIdleTimeOut::cIdleTimeOut(bool _kiosk) :
    QDialog(),
    ui(new Ui::idleTimeOut)
{
    DS << __PRETTY_FUNCTION__ << " : " << _kiosk << endl;
    idleTimeCnt = 0;
    ui->setupUi(this);
    setStyleSheet("background-color: yellow");
    if (_kiosk) {
        ui->labelUp->setText(trUtf8("A böngésző auomatikusan ki fog lépni"));
        ui->labelDown->setText(trUtf8("másodperc múlva.\n\nHa nem szertné, hogy a böngésző kilépjen,\nmegnyomhatja az ESC gombot is."));
        ui->contPB->setText(trUtf8("Ne!"));
        ui->endPB->setText(trUtf8("Kilépés."));
        waitTime = kioskIdleDialogTime;
    }
    else {
        ui->labelUp->setText(trUtf8("A terminál auomatikusan ki fog kapcsolni"));
        ui->labelDown->setText(trUtf8("másodperc múlva.\n\nHa nem szertné, hogy kikapcsoljon a terminál,\nmegnyomhatja az ESC gombot is."));
        ui->contPB->setText(trUtf8("Ne!"));
        ui->endPB->setText(trUtf8("Kikapcsolás!"));
        ui->lcdNumber->display(idleDialogTime);
        waitTime = idleDialogTime;
    }
    ui->lcdNumber->display(waitTime);
    timeCnt = 0;
    connect(ui->endPB,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->contPB, SIGNAL(clicked()), this, SLOT(reject()));
    timerId = startTimer(1000);
    // setModal(true);
}

cIdleTimeOut::~cIdleTimeOut()
{
    DS << __PRETTY_FUNCTION__ << endl;
//  killTimer(timerId);
    delete ui;
}

void cIdleTimeOut::timerEvent(QTimerEvent *)
{
    ++timeCnt;
    if (timeCnt >= waitTime) accept();
    int i = waitTime - timeCnt;
    setStyleSheet(QString("background-color: rgb(255,%1,0)").arg((255 * i) / waitTime));
    ui->lcdNumber->display(i);
}

