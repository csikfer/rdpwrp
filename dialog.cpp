#include "dialog.h"
#include "ui_dialog.h"
#include "QShortcut"

mainDialog * mainDialog::pItem = NULL;

mainDialog::mainDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dialog),
    domains(), servers(), rdpCmds(), rdpcmd(), offcmd(), rescmd(), hlpcmd(),
    browsercmd(),
    goNames(), goCommands(), goIcons(), goTimes(),
    procOut(), lastCommand()
{
    pItem = this;
    pToBox = NULL;
    pButtonGroup = NULL;
    procesStop = false;
    DS << __PRETTY_FUNCTION__ << endl;
    idleTimeCnt = 0;
    procTime = 0;
    timerId  = -1;
    pProc = NULL;
    ui->setupUi(this);
    ui->logOnPB->setDisabled(true);
    ui->clientNameL->setText(hostname);
    connect(ui->logOnPB,    SIGNAL(clicked()),            this, SLOT(logOn()));
    connect(ui->userLE,     SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->passwordLE, SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->domainCB,   SIGNAL(currentIndexChanged(int)),this,SLOT(selDomain(int)));

    connect(ui->browserPB,  SIGNAL(clicked()),            this, SLOT(browser()));

    connect(ui->offPB,      SIGNAL(clicked()),            this, SLOT(off()));
    connect(ui->rebootPB,   SIGNAL(clicked()),            this, SLOT(reboot()));
    connect(ui->cleanPB,    SIGNAL(clicked()),            this, SLOT(restart()));
    connect(ui->helpPB,     SIGNAL(clicked()),            this, SLOT(help()));

    QShortcut *pSc;
    pSc = new QShortcut(QKeySequence(QKeySequence::HelpContents), this);                // F1
    connect(pSc, SIGNAL(activated()), this, SLOT(help()));
    pSc = new QShortcut(QKeySequence(QKeySequence::InsertParagraphSeparator), this);    // Enter
    connect(pSc, SIGNAL(activated()), this, SLOT(enter()));
}

mainDialog::~mainDialog()
{
    DS << __PRETTY_FUNCTION__ << endl;
    pItem = NULL;
    killTimer(timerId);
    // if (pProc != NULL) delete pProc;
    delete ui;
}

void mainDialog::addDomain(QStringPair * pDom, QStringPairList *pServers)
{
    pItem->domains << pDom->first;
    QStringList srvs;
    QStringList cmds;
    foreach (QStringPair ss, *pServers) {
        srvs << ss.first;
        cmds << (ss.second.isEmpty() ? pDom->second : ss.second);
    }
    pItem->servers << srvs;
    pItem->rdpCmds << cmds;
    delete pDom;
    delete pServers;
}

inline QFrame *mainDialog::hLine()
{
    QFrame *pLine = new QFrame(this);
    pLine->setFrameShape(QFrame::HLine);
    return pLine;
}

inline QPushButton *mainDialog::button(QString& txt, QString ico)
{
    QPushButton *pPB = new QPushButton(this);
    // Ha felkiáltójellel kezdődik, akkor rejtett, csak a master user/psw -re jelenik meg.
    bool hidden = txt.startsWith(QChar('!'));
    if (hidden) {
        txt = txt.mid(1);
        pPB->hide();
    }
    pPB->setText(txt);
    pPB->setIcon(QIcon(ico));
    pPB->setIconSize(QSize(32,32));

    return pPB;
}

void mainDialog::set()
{
    ui->domainCB->clear();
    ui->domainCB->addItems(domains);
    ui->serverCB->clear();
    ui->serverCB->addItems(servers.first());
    QVBoxLayout *pLayout;
    if (isKiosk) {
        ui->offPB->hide();
    }
    if (isKiosk || idleTime < 0) {
        ui->autoOffSec->hide();
        ui->autoOffAfterL->setText(trUtf8("nincs"));
    }
    else ui->autoOffSec->setText(QString::number(idleTime));
    if (browsercmd.isEmpty()) {
        ui->browserPB->setDisabled(true);
        ui->browserPB->hide();
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
    }
    if (idleTime > 0) {
        connect(this, SIGNAL(doTOBox()), this, SLOT(idleTimeOutBox()), Qt::QueuedConnection);
        timerId = startTimer(1000);
    }
}

void mainDialog::timerEvent(QTimerEvent * pTe)
{
    DS << "Enter : " << __PRETTY_FUNCTION__ << endl;
    if (isDown) {
        DS << "isDown is on, drop timerEvent" << endl;
        return;
    }
    if (pToBox != NULL) {
        DS << "idleTimeOut is on, drop timerEvent" << endl;
        return;
    }
    if (timerId != pTe->timerId()) {
        DS << "Ismeretlen ID timerEvent-ben : " << pTe->timerId() << endl;
        return;
    }
    // Ha nem fut parancs / nem vagyunk háttérben, mérjuk a tétlenségi időt
    if (!runing()) {
        if (idleTime <= 0 || isKiosk) return;   // mégsem méricskélünk,
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


void    mainDialog::help()
{
    command(hlpcmd);
    idleTimeCnt = 0;
}

void    mainDialog::logOn()
{
    DS << __PRETTY_FUNCTION__ << endl;
    // Master-re megjelennek a rejtett gombok
    if (ui->userLE->text() == masterUser && ui->passwordLE->text() == masterPsw) {
        if (isKiosk) ui->offPB->show();
        if (pButtonGroup != NULL) {
            foreach (QAbstractButton *p, pButtonGroup->buttons()) {
                p->show();
            }
        }
        return;
    }
    QString dom = ui->domainCB->currentText();
    QString usr = ui->userLE->text();
    int i = usr.indexOf(QChar('\\'));
    if (i > 0) {
        dom = usr.mid(0, i);
        usr = usr.mid(i +1);
    }
    QString cmd = rdpcmd;
    // Alternatív parancs
    QString altCmd = rdpCmds.at(ui->serverCB->currentIndex()).at(ui->serverCB->currentIndex());
    if (altCmd.isEmpty() == false) cmd = altCmd;
    cmd = cmd
            .arg(dom)
            .arg(ui->serverCB->currentText())
            .arg(usr)
            .arg(ui->passwordLE->text());
    ui->passwordLE->clear();
    command(cmd);
}

void    mainDialog::enter()
{
    if (ui->logOnPB->isEnabled())           logOn();
    else if (ui->browserPB->isEnabled())    browser();
    else                                    help();
}

void    mainDialog::browser()
{
    command(browsercmd);
}

void mainDialog::doExit(const QString &cmd)
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
    if (cmd.isEmpty() == false) QProcess::execute(cmd);
    QApplication::exit(0);
}

void    mainDialog::off()
{
    doExit(offcmd);
}

void    mainDialog::reboot()
{
    doExit(rescmd);
}

void    mainDialog::restart()
{
    doExit();
}

void    mainDialog::go(int id)
{
    DS << __PRETTY_FUNCTION__ << endl;
    command(goCommands[id]);
}

void    mainDialog::chgUsrOrPw(QString)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->logOnPB->setDisabled(ui->userLE->text().isEmpty() || ui->passwordLE->text().isEmpty());
}

void    mainDialog::selDomain(int ix)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->serverCB->clear();
    ui->serverCB->addItems(servers[ix]);
    ui->serverCB->setCurrentIndex(0);
}

void mainDialog::command(const QString &cmd, int minTm)
{
    DS << "Command : " << cmd << endl; // törölni !!! jelszó lehet benne.
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    lastCmd = cmd;
#endif
    procesStop = false;
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
#if HIDEMAINIFRUNCHILD
    hide();
#endif
}

void    mainDialog::procStarted()
{
    DS << "proc started..." << endl;
    procTime = 0;
}

void    mainDialog::procReadyRead()
{
    DS << __PRETTY_FUNCTION__ << endl;
    if (isDown) return;
    QString s = QString::fromUtf8(pProc->readAll());
    procOut += s.replace(QChar('\n'), QString("\n<br>"));
}

void    mainDialog::procError(QProcess::ProcessError e)
{
    DS << __PRETTY_FUNCTION__ << (int)e << endl;
    if (isDown) return;
#if HIDEMAINIFRUNCHILD
#if MAINWINFULLSCREEN
    showFullScreen();
#else
    show();
#endif // MAINWINFULLSCREEN
#endif // HIDEMAINIFRUNCHILD

    if (procesStop) return;

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
    message(sWarn, msg);
}

void    mainDialog::procFinished(int r)
{
    DS << __PRETTY_FUNCTION__ << r << endl;
    if (isDown) return;
#if HIDEMAINIFRUNCHILD
#if MAINWINFULLSCREEN
    showFullScreen();
#else
    show();
#endif // MAINWINFULLSCREEN
#endif // HIDEMAINIFRUNCHILD
    if (procesStop) return;
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
        message(sWarn, msg);
    }
}

void    mainDialog::stopProc()
{
    procesStop = true;
    DS << "terminate program ..." << endl;
    pProc->terminate();
    pProc->waitForFinished(1000);
    DS << "terminated program." << endl;
}

void mainDialog::idleTimeOutBox()
{
    DS << "Enter : " << __PRETTY_FUNCTION__ << endl;
    if (isDown) return;
    if (pToBox != NULL) {
        QMessageBox::critical(this, "Kritikus hiba!", trUtf8("Program hiba : %1").arg(__PRETTY_FUNCTION__));
        ::exit(1);
    }
    activateWindow();
    repaint();
    pToBox = new cIdleTimeOut(isKiosk, this);
    int r = pToBox->exec();
    delete pToBox;
    pToBox = NULL;
    if (r == QDialog::Rejected) {
        DS << "Continue program..." << endl;
    }
    else {
        if (isKiosk) {
            stopProc();
        }
        else {
            DS << "off by " << __PRETTY_FUNCTION__ << endl;
            off();
            DS << "exit by " << __PRETTY_FUNCTION__ << endl;
            QApplication::exit();
        }
    }
    idleTimeCnt = 0;
    DS << "Leave : " << __PRETTY_FUNCTION__ << endl;
}

