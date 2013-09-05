#include "dialog.h"
#include <stdio.h>
#include <QDesktopWidget>
#include "tftp.h"

QTextStream *pDS = NULL;
int idleTimeCnt = 0;
QString sSugg;
QString sCrit;
QString sWarn;
QString hostname;
bool    isDown  = false;
bool    isKiosk = false;
int     desktopHeiht, desktopWidth;

int idleTime        = IDLETIME;
int idleDialogTime  = IDLEDIALOGTIME;
int minProgTime     = MINPRCTM;

inline void critical(QString msg)
{
    QMessageBox::critical(NULL, sCrit, msg + sSugg);
    ::exit(1);
}

inline QString nextArg(int& i)
{
    ++i;
    if (QApplication::arguments().size() <= i) critical(QObject::trUtf8("Hiányos atgumentum lista."));
    return QApplication::arguments().at(i);
}

int main(int argc, char *argv[])
{
    //QMyApplication a(argc, argv);
    QApplication a(argc, argv);
    // a.setQuitOnLastWindowClosed(false);

    sSugg = QObject::trUtf8("<br>Kérem forduljon a rendszergazdához!");
    sCrit = QObject::trUtf8("Végzetes hiba!");
    sWarn = QObject::trUtf8("Figyelmeztetés");
    desktopHeiht = QApplication::desktop()->height();
    desktopWidth = QApplication::desktop()->width();
    {
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

    QString confName = "rdpwrp.conf";
    QString tftpName;
    int n = QApplication::arguments().size();

    for (int i = 0; i < n; ++i) {
        QString arg = QApplication::arguments().at(i);
        if      (arg == "-c") confName     = nextArg(i);
        else if (arg == "-t") tftpName     = nextArg(i);
        else if (arg == "-l") localAddrStr = nextArg(i);
        else if (arg == "-4") ipProto = QAbstractSocket::IPv4Protocol;
        else if (arg == "-6") ipProto = QAbstractSocket::IPv6Protocol;
    }

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
    int r;
    while(!isDown) {
        if (!w.runing()) {
#if       MAINWINFULLSCREEN
            w.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
            w.showFullScreen();
#else  // MAINWINFULLSCREEN
            w.show();
#endif // MAINWINFULLSCREEN
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

void message(const QString& _t, const QString& _m)
{
    mainDialog *p = mainDialog::pItem;
    msgBox *m = new msgBox(p);
    m->setWindowTitle(_t);
    m->setText(_m);
    m->exec();
    delete m;
}
