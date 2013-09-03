#include "dialog.h"
#include "ui_dialog.h"
#include "idletimeout.h"

Dialog * Dialog::pItem = NULL;

Dialog::Dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dialog),
    domains(), servers(), rdpcmd(), offcmd(), rescmd(), hlpcmd(),
    kioskcmd(),
    goNames(), goCommands(), goIcons(), goTimes(),
    procOut(), lastCommand()
{
    pItem = this;
    pButtonGroup = NULL;
    DS << __PRETTY_FUNCTION__ << endl;
    idleTimeCnt = 0;
    procTime = 0;
    timerId  = -1;
    pProc = NULL;
    ui->setupUi(this);
    ui->logOnPB->setDisabled(true);
    ui->clientNameL->setText(hostname);
    connect(ui->logOnPB,    SIGNAL(clicked()),            this, SLOT(logOn()));
    connect(ui->helpPB,     SIGNAL(clicked()),            this, SLOT(help()));
    connect(ui->offPB,      SIGNAL(clicked()),            this, SLOT(off()));
    connect(ui->userLE,     SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->passwordLE, SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->domainCB,   SIGNAL(currentIndexChanged(int)),this,SLOT(selDomain(int)));

}

Dialog::~Dialog()
{
    DS << __PRETTY_FUNCTION__ << endl;
    pItem = NULL;
    killTimer(timerId);
    // if (pProc != NULL) delete pProc;
    delete ui;
}

inline QFrame *Dialog::hLine()
{
    QFrame *pLine = new QFrame(this);
    pLine->setFrameShape(QFrame::HLine);
    return pLine;
}
inline QPushButton *Dialog::button(QString txt, QString ico)
{
    QPushButton *pPB = new QPushButton(this);
    pPB->setText(txt);
    pPB->setIcon(QIcon(ico));
    pPB->setIconSize(QSize(32,32));
    return pPB;
}

void Dialog::set()
{
    ui->domainCB->clear();
    ui->domainCB->addItems(domains);
    ui->serverCB->clear();
    ui->serverCB->addItems(servers.first());
    QVBoxLayout *pLayout;
    if (isKiosk) {
        ui->offPB->setText(trUtf8("Újraindítás"));
        offcmd = rescmd;
        pLayout = ui->logOnLayout;
        pLayout->addWidget(hLine());
        QString icon = ":/images/info.ico";
        QPushButton *pPB = button(trUtf8("Kiosk (böngésző) indítása"), icon);
        pLayout->addWidget(pPB);
        connect(pPB, SIGNAL(clicked()), this, SLOT(kiosk()));
        ui->autoOffCnt->hide();
        ui->autoOffAfterL->setText(trUtf8("nincs."));
    }
    int n = goCommands.size();
    if (n > 0) {
        pLayout = ui->buttonLayout;
        pLayout->addWidget(hLine());
        pButtonGroup = new QButtonGroup(this);
        for (int i = 0; i < n; ++i) {
            QPushButton *pPB = button(goNames[i], goIcons[i]);
            pLayout->addWidget(pPB);
            pButtonGroup->addButton(pPB, i);
        }
        connect(pButtonGroup, SIGNAL(buttonReleased(int)), this, SLOT(go(int)));
        ui->autoOffCnt->setText(QString::number(idleTime));
    }
    connect(this, SIGNAL(doTOBox()), this, SLOT(idleTimeOutBox()), Qt::QueuedConnection);
    timerId = startTimer(1000);
}

void Dialog::timerEvent(QTimerEvent * pTe)
{
    DS << "Enter : " << __PRETTY_FUNCTION__ << endl;
    if (timerId != pTe->timerId()) {
        DS << "Ismeretlen ID timerEvent-ben : " << pTe->timerId() << endl;
        return;
    }
    // Ha nem fut parancs / nem vagyunk háttérben, mérjuk a tétlenségi időt
    if (isDown) return;
    if (pProc == NULL || pProc->state() == QProcess::NotRunning) {
        if (idleTime <= 0 || isKiosk) return;
        DS << "idle : " << idleTimeCnt << " no run" << endl;
        int maxCnt = idleTime - idleDialogTime;
        if (maxCnt <= idleTimeCnt) {
            idleTimeCnt = getIdleTime(maxCnt);
        }
        if (maxCnt <= idleTimeCnt) {
            emit doTOBox(); // idleTimeOutBox();
            idleTimeCnt = 0;
            DS << __PRETTY_FUNCTION__ << " exit (no run)" << endl;
            return;
        }
    }
    // Ha fut egy parancs, akkor annak a futási idejét mérjük
    else if (kioskIsOn) {
        DS << "idle : " << idleTimeCnt << " kiosk" << endl;
        int maxCnt = kioskIdleTime - kioskIdleDialogTime;
        if (maxCnt <= idleTimeCnt) {
            idleTimeCnt = getIdleTime(maxCnt);
        }
        if (maxCnt <= idleTimeCnt) {
            emit doTOBox(); // idleTimeOutBox();
            idleTimeCnt = 0;
            DS << __PRETTY_FUNCTION__ << " exit (kiosk)" << endl;
            return;
        }
    }
    else {
        ++procTime;
        DS << "idle : " << idleTimeCnt << " run " << procTime << endl;
        int maxCnt = idleTime - idleDialogTime;
        if (maxCnt <= idleTimeCnt) {
            idleTimeCnt = getIdleTime(maxCnt);
        }
        if (maxCnt <= idleTimeCnt) {
            emit doTOBox(); // idleTimeOutBox();
            idleTimeCnt = 0;
            DS << __PRETTY_FUNCTION__ << " exit (run)" << endl;
            return;
        }
    }
    ++idleTimeCnt;
    DS << __PRETTY_FUNCTION__ << " up" << endl;
}


void    Dialog::help()
{
    command(hlpcmd);
    idleTimeCnt = 0;
}

void    Dialog::logOn()
{
    DS << __PRETTY_FUNCTION__ << endl;
    QString cmd = rdpcmd
            .arg(ui->domainCB->currentText())
            .arg(ui->serverCB->currentText())
            .arg(ui->userLE->text())
            .arg(ui->passwordLE->text());
    ui->passwordLE->clear();
    command(cmd);
}

void    Dialog::kiosk()
{
    kioskIsOn = true;
    command(kioskcmd, -1);
}

void    Dialog::off()
{
    DS << __PRETTY_FUNCTION__ << endl;
    isDown = true;
    if (pProc != NULL) {
        disconnect(pProc, SIGNAL(readyRead()),                     this, SLOT(procReadyRead()));
        disconnect(pProc, SIGNAL(error(QProcess::ProcessError)),   this, SLOT(procError(QProcess::ProcessError)));
        disconnect(pProc, SIGNAL(finished(int)),                   this, SLOT(procFinished(int)));
        disconnect(pProc, SIGNAL(started()),                       this, SLOT(procStarted()));
        if (pProc->state() != QProcess::NotRunning) {
            pProc->kill();
            pProc->waitForFinished(1000);
        }
    }
    command(offcmd, -1);
}

void    Dialog::go(int id)
{
    DS << __PRETTY_FUNCTION__ << endl;
    command(goCommands[id]);
}

void    Dialog::chgUsrOrPw(QString)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->logOnPB->setDisabled(ui->userLE->text().isEmpty() || ui->passwordLE->text().isEmpty());
}

void    Dialog::selDomain(int ix)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->serverCB->clear();
    ui->serverCB->addItems(servers[ix]);
    ui->serverCB->setCurrentIndex(0);
}

void Dialog::command(const QString &cmd, int minTm)
{
    DS << "Command : " << cmd << endl;
    if (pProc != NULL) {
        if (pProc->state() != QProcess::NotRunning && isDown == false) {
            QMessageBox::critical(this, "Kritikus hiba!", trUtf8("Program hiba : %1").arg(__PRETTY_FUNCTION__));
            ::exit(1);
        }
    }
    else {
        pProc = new QProcess(this);
        if (isDown == false) {
            pProc->setProcessChannelMode(QProcess::MergedChannels);
            connect(pProc, SIGNAL(readyRead()),                     this, SLOT(procReadyRead()));
            connect(pProc, SIGNAL(error(QProcess::ProcessError)),   this, SLOT(procError(QProcess::ProcessError)));
            connect(pProc, SIGNAL(finished(int)),                   this, SLOT(procFinished(int)));
            connect(pProc, SIGNAL(started()),                       this, SLOT(procStarted()));
        }
    }
    actMinProgTime = minTm;
    procOut.clear();
    procTime = -1;
    pProc->start(cmd, QIODevice::ReadOnly);
    hide();
}

void    Dialog::procStarted()
{
    DS << "proc started..." << endl;
    procTime = 0;
}

void    Dialog::procReadyRead()
{
    DS << __PRETTY_FUNCTION__ << endl;
    if (isDown) return;
    QString s = QString::fromUtf8(pProc->readAll());
    if (kioskIsOn) return;
    procOut += s.replace(QChar('\n'), QString("\n<br>"));
}

void    Dialog::procError(QProcess::ProcessError e)
{
    DS << __PRETTY_FUNCTION__ << (int)e << endl;
    if (isDown) return;
#if FULLSCREEN
    showFullScreen();
#else
    show();
#endif // FULLSCREEN

    if (kioskIsOn) {
        kioskIsOn = false;
        return;
    }

    QString msg;
    msg  = trUtf8("<h1>Parancs, vagy parancs idítási hiba</h1>\n");
    switch (e) {
    case QProcess::FailedToStart:   msg += trUtf8("A program idítása nem lehetséges.\n"); break;
    case QProcess::Crashed:         msg += trUtf8("A program összeomlott.\n"); break;
    case QProcess::Timedout:        msg += trUtf8("Idő tullépés.\n"); break;
    case QProcess::WriteError:      msg += trUtf8("Program írási hiba.\n"); break;
    case QProcess::ReadError:       msg += trUtf8("Program olvasási hiba.\n"); break;
    case QProcess::UnknownError:
    default:                        msg += trUtf8("Ismeretlen hiba.\n"); break;
    }
    msg += trUtf8("<h2>Parancs sor:</h2>\n");
    msg += lastCommand;
    if (procOut.isEmpty()) {
        msg += trUtf8("<h2>A programnak nem volt kimenete.</h2>\n");
    }
    else {
        msg += trUtf8("<h2>A program kimenete:</h2>\n");
        msg += procOut + "\n";
    }
    message(trUtf8("Hiba"), msg);
}

void    Dialog::procFinished(int r)
{
    DS << __PRETTY_FUNCTION__ << r << endl;
    if (isDown) return;
#if FULLSCREEN
    showFullScreen();
#else
    show();
#endif // FULLSCREEN
    if (kioskIsOn) {
        kioskIsOn = false;
        return;
    }
    if (procTime < actMinProgTime) {  // Túl hamar kilépett
        QString msg;
        msg  = trUtf8("<h1>A parancs futási ideje gyanús. Hiba történt?</h1>\n");
        msg += trUtf8("<h2>Kilépési kód : %1").arg(r);
        if (procOut.isEmpty()) {
            msg += trUtf8("<h2>A programnak nem volt kimenete.</h2>\n");
        }
        else {
            msg += trUtf8("<h2>A program kimenete:</h2>\n");
            msg += procOut + "\n";
        }
        message(trUtf8("Figyelmeztetés"), msg);
    }
}

void Dialog::idleTimeOutBox()
{
    if (isDown) return;
    DS << "kill timer..." << endl;
    killTimer(timerId);
    timerId = -1;
    if (isKiosk) {
        DS << __PRETTY_FUNCTION__ << " kiosk" << endl;
        cIdleTimeOut    d(true, this);
        if (d.exec() == QDialog::Rejected) {
            idleTimeCnt = 0;
            DS << "Continue kiosk browser..." << endl;
            // Ez után, ha kilép a föggvényből
            // akkor kilép az app event loop-ból is, vagyis kilép a program.
            // De miért ??
        }
        else {
            DS << "terminate browser ..." << endl;
            pProc->terminate();
            pProc->waitForFinished(1000);
            DS << "terminated browser." << endl;
            kioskIsOn = false;
        }
    }
    else {
        DS << __PRETTY_FUNCTION__ <<  endl;
        cIdleTimeOut    d(false, this);
        if (d.exec() == QDialog::Rejected) {
            idleTimeCnt = 0;
            DS << "Continue program ..." << endl;
        }
        else {
            DS << "off by " << __PRETTY_FUNCTION__ << endl;
            off();
            DS << "exit by " << __PRETTY_FUNCTION__ << endl;
            QApplication::exit();
            return;
        }
    }
    DS << "Start timer..." << endl;
    timerId = startTimer(1000);
    DS << __PRETTY_FUNCTION__ << " end function" << endl;
}

