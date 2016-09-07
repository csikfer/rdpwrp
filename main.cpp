#include "dialog.h"
#include <stdio.h>
#include <QDesktopWidget>
#include "tftp.h"
#include "control.h"
#include "parser.h"

bool mainIsFullScreen = true;
QTextStream *pDS = NULL;
int idleTimeCnt = 0;
QString sSugg;
QString sCrit;
QString sWarn;
QString hostname;
QString localAddrStr;
QHostAddress localAddr;
bool    isDown  = false;
bool    isKiosk = false;
int     desktopHeight, desktopWidth;

int idleTime        = IDLETIME;
int idleDialogTime  = IDLEDIALOGTIME;
int minProgTime     = MINPRCTM;

QString getKernelParamValue(const QString& name, const QString& def)
{
    QFile cmdline("/proc/cmdline");
    if (cmdline.open(QIODevice::ReadOnly)) {
        QString params = cmdline.readAll();
        foreach (QString par, params.split(QChar(' '), QString::KeepEmptyParts)) {
            QStringList nv = par.split(QChar('='));
            if (nv.size() == 2 && nv.at(0) == name) {
                DS << "Find pernel param : " << par << endl;
                return nv.at(1);
            }
        }
    }
    else {
        DS << "Nem tudom megnyitni a /proc/cmdline fájlt !!" << endl;
    }
    return def;
}

inline QString nextArg(int& i)
{
    ++i;
    if (QApplication::arguments().size() <= i) critical(QObject::trUtf8("Hiányos argumentum lista."));
    return QApplication::arguments().at(i);
}

inline int nextArgUInt(int& i)
{
    bool ok = false ;
    QString s = nextArg(i);
    unsigned int r = s.toUInt(&ok);
    if (!ok) critical(QObject::trUtf8("Nem numerikus argumentum : \"%1\"").arg(s));
    return r;
}

static void setLocalAddr()
{
    if (localAddrStr.isEmpty()) {   // Nincs megadva lokális cím, kitaláljuk
        QList<QHostAddress>  aa = QNetworkInterface::allAddresses();
        foreach (QHostAddress a, aa) {
            if (a.isNull()) continue;
            if (ipProto != QAbstractSocket::UnknownNetworkLayerProtocol) {
                if (ipProto != a.protocol()) continue;
            }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            if (a.isLoopback()) continue;
#else
            QString as = a.toString();
            if (as.indexOf("127.") == 0) continue;
            if (as == "::1") continue;
#endif
            localAddr = a;
            break;
        }
        if (localAddr.isNull()) {
            critical(QObject::trUtf8("Nem állapítható meg a saját gép ip címe"));
        }
    }
    else {
        localAddr.setAddress(localAddrStr);
        if (localAddr.isNull()) {
            critical(QObject::trUtf8("A megadott saját cím értelmezhetetlen : %1").arg(localAddrStr));
        }
    }
}

/****************************************** main() ******************************************/
int main(int argc, char *argv[])
{
    //QMyApplication a(argc, argv);
    QApplication a(argc, argv);
    // a.setQuitOnLastWindowClosed(false);

    sSugg = QObject::trUtf8("<br>Kérem forduljon a rendszergazdához!");
    sCrit = QObject::trUtf8("Végzetes hiba!");
    sWarn = QObject::trUtf8("Figyelmeztetés");
    desktopHeight = QApplication::desktop()->height();
    desktopWidth  = QApplication::desktop()->width();
    // Saját hoszt név
    const char * pe = getenv("HOSTNAME");
    if (pe == NULL || (hostname = pe).isEmpty()) {
        QFile   fhn("/etc/hostname");
        if (fhn.open(QIODevice::ReadOnly)) {
            hostname = fhn.readAll();
        }
        else {
            hostname = QObject::trUtf8("Ismeretlen");
        }
    }
#if __DEBUG
    pDS = new QTextStream(stderr, QIODevice::WriteOnly);
#if 0  // TEST
    DS << "args : " << QApplication::arguments().join(",") << endl;
    int test = getIdleTime();
    DS << "idle time : " << test << endl;
    ::exit(0);
#endif // END TEST
#else // __DEBUG
    pDS = new QTextStream(fopen("/dev/null", "w"), QIODevice::WriteOnly);
#endif // __DEBUG

    // Ha megadjuk kernel paraméterként az rdpwrp-ta akkor annak az értéke lessz a konfig állomány neve
    // A -c pakcsoló fellülbírálja !
    QString confName = getKernelParamValue(STR(APPNAME), STR(APPNAME) ".conf");
    QString tftpName;
    int n = QApplication::arguments().size();
    // Program kapcsolók
    for (int i = 0; i < n; ++i) {
        QString arg = QApplication::arguments().at(i);
        if      (arg == "-c") confName     = nextArg(i);                // Konfig fájl neve
        else if (arg == "-t") tftpName     = nextArg(i);                // tftp szerver címe, ahonnan a konfig letöltendő
        else if (arg == "-l") localAddrStr = nextArg(i);                // saját cím
        else if (arg == "-4") ipProto = QAbstractSocket::IPv4Protocol;  // csak IPV4 használata (ha nincs -l)
        else if (arg == "-6") ipProto = QAbstractSocket::IPv6Protocol;  // csak IPV6 használata (ha nincs -l)
        else if (arg == "-p") cmdPort = nextArgUInt(i);                 // UDP parancs port száma
        else if (arg == "-F") mainIsFullScreen = false;                 // Nem full screen / test
    }

    setLocalAddr();

    QIODevice *pIn      = NULL;
    QByteArray*pInArray = NULL;

    if (tftpName.isEmpty()) {  // Lokális konfig
        pIn = new QFile(confName);
    }
    else {                      // Konfig egy tftp szerverről
        QTFtpClient tftp(tftpName);
        if (!tftp) critical(tftp.lastError());
        pInArray = new QByteArray;
        if (!tftp.getByteArray(confName, pInArray)) critical(tftp.lastError());
        pIn = new QBuffer(pInArray);
    }
    if (!pIn->open(QIODevice::ReadOnly)) {
        critical(QObject::trUtf8("A konfigurációs fájl %1, nem olvasható, vagy nem létezik.").arg(confName));
    }
    pIn->seek(0);

    mainDialog w;

    if (0 != parseConfig(pIn)) {
        const QString br = "<br>";
        QString msg = QObject::trUtf8("A konfigurációs állomány tartalma nem értelmezhető.");
        if (tftpName.isEmpty() == false) confName = tftpName + QChar(':') + confName;
        msg += QObject::trUtf8("<br>A %1 állomány %2 sor %3 oszlopában :")
                .arg(confName)
                .arg(yyLineNo)
                .arg(yyLastLine.size() - yyLine.size());
        msg += br + yyLastError;
        msg += br + QObject::trUtf8("A hibás szöveg sor:");
        msg += br + "<i>" + yyLastLine + "</i>";
        critical(msg);
    }
    delete pIn;
    if (pInArray != NULL) delete pInArray;

    w.set();

    cCntrl  rControl;   // Távvezérlés indítása

    int r;
    while(!isDown) {
        if (!w.runing()) {
            if (mainIsFullScreen) {
                w.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
                w.showFullScreen();
            }
            else {
                w.show();
            }
        }
        DS << "start event loop : " << endl;
        r = a.exec();
        DS << "exit event loop : " << r << endl;
    };
    if (w.runing()) w.stopProc();
    return r;
}

/* ***************************************************************** *

QMyApplication::QMyApplication(int argc, char *argv[]) : QApplication(argc, argv)
{
    idleTimeCnt = 0;
}

bool QMyApplication::notify ( QObject * receiver, QEvent * event )
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::KeyPress) {
        idleTimeCnt = 0;
    }
    return QApplication::notify(receiver, event);
}

 * ***************************************************************** */

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
    int y = desktopHeight/6;
    int w = (desktopWidth*2)/3;
    int h = (desktopHeight*2)/3;
    setGeometry(x, y, w, h);
    timer = startTimer(300 * 1000);  // 5 perc mulva automatikusan becsukódik
}

void    msgBox::timerEvent(QTimerEvent *)
{
    accept();
}

void message(const QString& _t, const QString& _m)
{
    mainDialog *p = mainDialog::pItem;
    msgBox *m = new msgBox(p);
    m->setWindowTitle(_t);
    m->setText(_m);
    m->exec();
    delete m;
}
