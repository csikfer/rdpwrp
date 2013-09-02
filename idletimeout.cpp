#include "idletimeout.h"
#include <QPushButton>
#include "dialog.h"

cIdleTimeOut::cIdleTimeOut(bool _kiosk, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::idleTimeOut)
{
    idleTimeCnt = 0;
    ui->setupUi(this);
    setStyleSheet("background-color: yellow");
    if (_kiosk) {
        ui->labelUp->setText(trUtf8("A böngésző auomatikusan ki fog lépni"));
        ui->labelDown->setText(trUtf8("másodperc múlva.\n\nHa nem szertné, hogy a böngésző kilépjen,\nmegnyomhatja az ESC gombot is."));
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(trUtf8("Ne!"));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(trUtf8("Kilépés."));
        waitTime = kioskIdleDialogTime;
    }
    else {
        ui->labelUp->setText(trUtf8("A terminál auomatikusan ki fog kapcsolni"));
        ui->labelDown->setText(trUtf8("másodperc múlva.\n\nHa nem szertné, hogy kikapcsoljon a terminál,\nmegnyomhatja az ESC gombot is."));
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(trUtf8("Ne!"));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(trUtf8("Kikapcsolás!"));
        ui->lcdNumber->display(idleDialogTime);
        waitTime = idleDialogTime;
    }
    ui->lcdNumber->display(waitTime);
    timeCnt = 0;
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    timerId = startTimer(1000);
    // setModal(true);
}

cIdleTimeOut::~cIdleTimeOut()
{
    killTimer(timerId);
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

