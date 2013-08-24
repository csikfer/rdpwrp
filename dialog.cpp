#include "dialog.h"
#include "ui_dialog.h"
#include "idletimeout.h"
#include <QTextEdit>
#include <QDialogButtonBox>

Dialog::Dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dialog),
    domains(), servers(), rdpcmd(), offcmd(), goNames(), goCommands(),
    procOut(), lastCommand()
{
    DS << __PRETTY_FUNCTION__ << endl;
    idleTime = 0;
    procTime = 0;
    timerId  = -1;
    pProc = NULL;
    ui->setupUi(this);
    ui->logOnPB->setDisabled(true);
    ui->clientNameL->setText(hostname);
    connect(ui->logOnPB,    SIGNAL(clicked()),            this, SLOT(logOn()));
    connect(ui->goPB,       SIGNAL(clicked()),            this, SLOT(go()));
    connect(ui->userLE,     SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->passwordLE, SIGNAL(textChanged(QString)), this, SLOT(chgUsrOrPw(QString)));
    connect(ui->domainCB,   SIGNAL(currentIndexChanged(int)),this,SLOT(selDdomain(int)));

    timerId = startTimer(1000);
}

Dialog::~Dialog()
{
    DS << __PRETTY_FUNCTION__ << endl;
    killTimer(timerId);
    if (pProc != NULL) delete pProc;
    delete ui;
}

void Dialog::timerEvent(QTimerEvent * pTe)
{
    if (timerId != pTe->timerId()) {
        DS << "Ismeretlen ID timerEvent-ben : " << pTe->timerId() << endl;
        return;
    }
    // Ha nem fut parancs / nem vagyunk háttérben, mérjuk a tétlenségi időt
    if (pProc == NULL || pProc->state() == QProcess::NotRunning) {
        ui->autoOffCnt->setText(QString::number(IDLETIME - idleTime));
        if ((IDLETIME - IDLEDIALOGTIME) <= idleTime) {
            idleTimeOut();
        }
        ++idleTime;
        procTime = 0;
    }
    // Ha fut egy parancs, akkor annak a futási idejét mérjük
    else {
        ++procTime;
        idleTime = 0;
    }
}

/// A domain-eket és a hívható terminálszervereket felsoroló fájl felolvasáas
/// A fájlban az ü#include <QProcess>res, vagy csak space karaktereket tartalmazó sorok figyelmen kívül lesznek hagyva.
/// A domain név bevezető space karakterek nélkül adandó meg, és ezt követik a szerver nevek (legalább egy)
/// melyek esetén a sor mindíg egy space kerekterrel kezdődik. Pl.:
/// @code
/// domain1
///     server1.domain1
///     server2.domain2
/// domain2
///     server1.domain2
/// @endcode
bool Dialog::rdDomains(QFile& fDomains)
{
    DS << __PRETTY_FUNCTION__ << endl;
    QByteArray l;
    QString s;
    while ((l = fDomains.readLine()).size() > 0) {
        s = QString::fromUtf8(l);
        if (s.simplified().isEmpty()) continue;
        if (s[0].isSpace()) {   // Szerver a domain-ban (első karakter egy space, tab stb.)
            // Hi nincs még domain, akkor az gáz
            if (domains.isEmpty()) return false;
            servers.last() << s.simplified();
        }
        // Domain megadása (a domain neve a sor eleján kezdődik
        else {
            // Ha az előző domain-hez nincs megadva szerver az nem jó
            if ((!domains.isEmpty()) && servers.last().isEmpty()) return false;
            domains << s.simplified();  // Domain a listába
            servers << QStringList();   // Az egyenlőre öres szerver lista a domain-hez
        }
    }
    if (domains.isEmpty() || servers.size() != domains.size() || servers.last().isEmpty()) {
        return false;
    }
    ui->domainCB->addItems(domains);
    ui->serverCB->addItems(servers.first());
    return true;
}
/// A hívható parancsokat definiáló fájl felhome.filePath("commands")dolgozása
/// A parancs lista fájl minden sora két mezőből áll, a szeparátor a '|' karakter.
/// Az üres ill. csak space-t tartalmazó sorok figyelmen kívül lsznek hagyva.
/// Az első mezó a comboBoxban kiválaszható név, a második a hozzá tartozó parancs.
/// Két kitüntetett nevet a '!' és a '!!' kötelezü megadni, ezek nem a comboBox-ban jelenek meg.
/// A '!' az RDP parancsot definiálja, ahol a %1 a domain név, a %2 szerver név,
/// %3 user név, és a %4 a jelszó.
/// A '!!' pedig a kikapcsoló parancsot, mely akkor kerül meghívásra, ha 5 percig nem történik semmi.
/// A második mezőben megadba a '!!' stringet, az a '!!' után megadott paranccsal azonos parancsot jelent.
/// pl.:
/// @code
/// !|/usr/bin/xfreerdp --ignore-certificate -f -z -a24 --plugin rdpsnd --plugin rdpdr --data disk:USB:/media/root -- -d %1 -u %3 -p %4 %2
/// !!|/usr/bin/sudo /sbin/poweroff
////// Terminál kikapcsolása|!!
/// Böngésző indítása|/usr/bin/chromium-browser
/// @endcode
bool Dialog::rdCommands(QFile& fCommands)
{
    DS << __PRETTY_FUNCTION__ << endl;
    QByteArray  l;
    QString     s;
    QStringList sl;
    while ((l = fCommands.readLine()).size() > 0) {
        s = QString::fromUtf8(l);
        s = s.simplified();
        if (s.isEmpty()) continue;
        sl = s.split(QChar('|'));
        // 2 db nem üres mezőre számítunk.
        if (sl.size() != 2 || sl.first().isEmpty() || sl[1].isEmpty()) return false;
        if (sl.first() == QString("!")) {   // Az RDP parancs (minta)
            rdpcmd = sl[1];
        }
        else if (sl.first() == QString("!!")) {  // Kikapcsolás parancs
            offcmd = sl[1];
        }
        else {
            goNames << sl[0];
            QString c = sl[1];
            if (c == QString("!!")) {
                if (offcmd.isEmpty()) return false;
                c = offcmd;
            }
            goCommands << c;
        }
    }
    // A kötelező parancsok megadva?
    if (rdpcmd.isEmpty()) return false;
    if (offcmd.isEmpty()) return false;
    // Ha nincs parancs lista, akkor letiltjuk a hozzátartozó elemeket
    if (goNames.isEmpty()) {
        ui->goCB->setDisabled(true);
        ui->goPB->setDisabled(true);
    }
    // megadjuk a hivható parancsok név listáját
    else {
        ui->goCB->addItems(goNames);
    }
    return true;
}

void    Dialog::go()
{
    DS << __PRETTY_FUNCTION__ << endl;
    int i = ui->goCB->currentIndex();
    command(goCommands[i]);
    idleTime = 0;
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

void    Dialog::chgUsrOrPw(QString)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->logOnPB->setDisabled(ui->userLE->text().isEmpty() || ui->passwordLE->text().isEmpty());
}

void    Dialog::selDdomain(int ix)
{
    DS << __PRETTY_FUNCTION__ << endl;
    ui->serverCB->clear();
    ui->serverCB->addItems(servers[ix]);
    ui->serverCB->setCurrentIndex(0);
}

void Dialog::command(const QString &cmd)
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

    if (procTime < MINPRCTM) {  // Túl hamar kilépett
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
    QDialog *m = new QDialog(this);
    m->setWindowTitle(_t);
    QVBoxLayout *l = new QVBoxLayout(m);
    // m->setLayout(l);
    QTextEdit *t = new QTextEdit(m);
    t->setReadOnly(true);
    t->setText(_m);
    l->addWidget(t);
    QDialogButtonBox *b = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, m);
    l->addWidget(b);
    connect(b, SIGNAL(accepted()), m, SLOT(accept()));
    int x = desktopWidth/6;
    int y = desktopHeiht/6;
    int w = (desktopWidth*2)/3;
    int h = (desktopWidth*2)/3;
    m->setGeometry(x, y, w, h);
    m->exec();
    delete m;
}

void Dialog::idleTimeOut()
{
    cIdleTimeOut    d(this);
    if (d.exec() == QDialog::Rejected) {
        idleTime = 0;
        return;
    }
    command(offcmd);
    exit(1);
}
