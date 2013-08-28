#include "dialog.h"
#include "ui_dialog.h"
#include "idletimeout.h"

int idleTime        = IDLETIME;
int idleDialogTime  = IDLEDIALOGTIME;
int minProgTime     = MINPRCTM;

Dialog * Dialog::pItem = NULL;

Dialog::Dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dialog),
    domains(), servers(), rdpcmd(), offcmd(), hlpcmd(),
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

    timerId = startTimer(1000);
}

Dialog::~Dialog()
{
    pItem = NULL;
    DS << __PRETTY_FUNCTION__ << endl;
    killTimer(timerId);
    if (pProc != NULL) delete pProc;
    delete ui;
}

void Dialog::set()
{
    ui->domainCB->clear();
    ui->domainCB->addItems(domains);
    ui->serverCB->clear();
    ui->serverCB->addItems(servers.first());
    int n = goCommands.size();
    if (n > 0) {
        QVBoxLayout *pLayout = ui->buttomLayout;
        pButtonGroup = new QButtonGroup(this);
        QFrame *pLine = new QFrame(this);
        pLine->setFrameShape(QFrame::HLine);
        pLayout->addWidget(pLine);
        for (int i = 0; i < n; ++i) {
            QPushButton *pPB = new QPushButton(this);
            pPB->setText(goNames[i]);
            pPB->setIcon(QIcon(goIcons[i]));
            pPB->setIconSize(QSize(32,32));
            pLayout->addWidget(pPB);
            pButtonGroup->addButton(pPB, i);
        }
        connect(pButtonGroup, SIGNAL(buttonReleased(int)), this, SLOT(go(int)));
    }

}

void Dialog::timerEvent(QTimerEvent * pTe)
{
    if (timerId != pTe->timerId()) {
        DS << "Ismeretlen ID timerEvent-ben : " << pTe->timerId() << endl;
        return;
    }
    // Ha nem fut parancs / nem vagyunk háttérben, mérjuk a tétlenségi időt
    if (pProc == NULL || pProc->state() == QProcess::NotRunning) {
        ui->autoOffCnt->setText(QString::number(IDLETIME - idleTimeCnt));
        if ((IDLETIME - IDLEDIALOGTIME) <= idleTimeCnt) {
            idleTimeOut();
        }
        ++idleTimeCnt;
        procTime = 0;
    }
    // Ha fut egy parancs, akkor annak a futási idejét mérjük
    else {
        ++procTime;
        idleTimeCnt = 0;
    }
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

void    Dialog::off()
{
    command(offcmd);
}

void    Dialog::go(int id)
{
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
        if (pProc->state() != QProcess::NotRunning) {
            QMessageBox::critical(this, "Kritikus hiba!", trUtf8("Program hiba : %1").arg(__PRETTY_FUNCTION__));
            exit(1);
        }
    }
    else {
        pProc = new QProcess(this);
        pProc->setProcessChannelMode(QProcess::MergedChannels);
        connect(pProc, SIGNAL(readyRead()),                     this, SLOT(procReadyRead()));
        connect(pProc, SIGNAL(error(QProcess::ProcessError)),   this, SLOT(procError(QProcess::ProcessError)));
        connect(pProc, SIGNAL(finished(int)),                   this, SLOT(procFinished(int)));
        connect(pProc, SIGNAL(started()),                       this, SLOT(procStated()));
    }
    actMinProgTime = minTm;
    procOut.clear();
    procTime = 0;
    pProc->start(cmd, QIODevice::ReadOnly);
    hide();
}

void    Dialog::procStated()
{
    DS << "proc started..." << endl;
}

void    Dialog::procReadyRead()
{
    DS << __PRETTY_FUNCTION__ << endl;
    QString s = QString::fromUtf8(pProc->readAll());
    procOut += s.replace(QChar('\n'), QString("\n<br>"));
}

void    Dialog::procError(QProcess::ProcessError e)
{
    DS << __PRETTY_FUNCTION__ << endl;
    showFullScreen();

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
    DS << __PRETTY_FUNCTION__ << endl;
    showFullScreen();

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

void Dialog::message(const QString& _t, const QString& _m)
{
    msgBox *m = new msgBox(this);
    m->setWindowTitle(_t);
    m->setText(_m);
    m->exec();
    delete m;
}

void Dialog::idleTimeOut()
{
    cIdleTimeOut    d(this);
    if (d.exec() == QDialog::Rejected) {
        idleTimeCnt = 0;
        return;
    }
    off();
    // exit(1); // csak elfedi, ha hiba van
}

msgBox::msgBox(QWidget *p) : QDialog(p)
{
    layout = new QVBoxLayout(this);
    // setLayout(layout);
    text = new QTextEdit(this);
    text->setReadOnly(true);
    layout->addWidget(text);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    int x = desktopWidth/6;
    int y = desktopHeiht/6;
    int w = (desktopWidth*2)/3;
    int h = (desktopHeiht*2)/3;
    setGeometry(x, y, w, h);
    timer = startTimer(300 * 1000);  // 5 perc mulva automatikusan becsukódik
}

void    msgBox::timerEvent(QTimerEvent *)
{
    accept();
}

