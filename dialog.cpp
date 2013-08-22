#include "dialog.h"
#include "ui_dialog.h"
#include <QProcess>


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->logOnPB->setDisabled(true);
    connect(ui->logOnPB,    SIGNAL(clicked()),            this, SLOT(logOn()));
    connect(ui->goPB,       SIGNAL(clicked()),            this, SLOT(go()));
    connect(ui->userLE,     SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
    connect(ui->passwordLE, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
    connect(ui->domainCB,   SIGNAL(currentIndexChanged(int)),this,SLOT(setdomain(int)));

}

Dialog::~Dialog()
{
    delete ui;
}

/// A domain-eket és a hívható terminálszervereket felsoroló fájl felolvasáas
/// A fájlban az üres, vagy csak space karaktereket tartalmazó sorok figyelmen kívül lesznek hagyva.
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
            domains << s.simplified();
            servers << QStringList();
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
    QByteArray  l;
    QString     s;
    QStringList sl;
    while ((l = fCommands.readLine()).size() > 0) {
        s = QString::fromUtf8(l);
        s = s.simplified();
        if (s.isEmpty()) continue;
        sl = s.split(QChar('|'));
        // Momentán 2 mezőre számítunk.
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
    if (rdpcmd.isEmpty()) return false;
    if (offcmd.isEmpty()) return false;
    if (goNames.isEmpty()) {
        ui->goCB->setDisabled(true);
        ui->goPB->setDisabled(true);
    }
    else {
        ui->goCB->addItems(goNames);
    }
    return true;
}

void Dialog::idleTime(int cnt)
{
    ui->autoOffCnt->setText(QString::number(cnt));
    if (cnt <= 0) {
        command(offcmd);
    }
}

void    Dialog::go()
{
    int i = ui->goCB->currentIndex();
    command(goCommands[i]);
}

void    Dialog::logOn()
{
    QString cmd = rdpcmd
            .arg(ui->domainCB->currentText())
            .arg(ui->serverCB->currentText())
            .arg(ui->userLE->text())
            .arg(ui->passwordLE->text());
    command(cmd);
}

void    Dialog::changed(QString)
{
    ui->logOnPB->setDisabled(ui->userLE->text().isEmpty() || ui->passwordLE->text().isEmpty());
}

void    Dialog::setdomain(int ix)
{
    ui->serverCB->clear();
    ui->serverCB->addItems(servers[ix]);
    ui->serverCB->setCurrentIndex(0);
}

void Dialog::command(const QString &cmd)
{
    DS << "Command : " << cmd << endl;
    if (QProcess::startDetached(cmd)) {
        exit(0);
    }
    else {
        QMessageBox::warning(this, "Hiba", trUtf8("Starting : \"%1\"").arg(cmd));
    }
}
