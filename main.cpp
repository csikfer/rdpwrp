#include "dialog.h"
#include <stdio.h>
#include <QDesktopWidget>

QTextStream *pDS = NULL;
int idleTimeCnt = 0;
QString sSugg;
QString sCrit;
QString hostname;
bool    isKiosk = false;
int     desktopHeiht, desktopWidth;

int main(int argc, char *argv[])
{
    QMyApplication a(argc, argv);
    //A resource-ban lévő ikonom
    sSugg = QObject::trUtf8("<br>Kérem forduljon a rendszergazdához!");
    sCrit = QObject::trUtf8("Végzetes hiba!");
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
#ifdef __DEBUG
    pDS = new QTextStream(stderr, QIODevice::WriteOnly);
    // TEST
    QString arg1;
    if (argc > 1) arg1 = argv[1];
    if (arg1 == QString("get-idle")) {
        int test = getIdleTime();
        DS << "idle time : " << test << endl;
        exit(0);
    }
    // END TEST
#else
    pDS = new QTextStream(fopen("/dev/null", "w"), QIODevice::WriteOnly);
#endif

    Dialog w;

    QFile    fconf(QString("./.%1.conf").arg(STR(APPNAME)));

    if (!fconf.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(NULL, sCrit, QObject::trUtf8("A konfigurációs fájl nem olvasható, vagy nem létezik.") + sSugg);
        exit(1);
    }
    if (0 != parseConfig(&fconf)) {
        const QString br = "<br>";
        QString msg = QObject::trUtf8("A konfigurációs állomány tartalma nem értelmezhető.");
        msg += QObject::trUtf8("<br>A %1 állomány %2 sor %3 oszlopában :")
                .arg(fconf.fileName())
                .arg(yyLineNo)
                .arg(yyLastLine.size() - yyLine.size());
        msg += br + yyLastError;
        msg += br + QObject::trUtf8("A hibás szöveg sor:");
        msg += br + "<i>" + yyLastLine + "</i>";
        msg += sSugg;
        QMessageBox::critical(NULL, sCrit,  msg);
        exit(1);
    }
    w.set();

    w.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    w.showFullScreen();
    
    return a.exec();
}

/* ***************************************************************** */

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

/* ***************************************************************** */

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
    Dialog *p = Dialog::pItem;
    msgBox *m = new msgBox(p);
    m->setWindowTitle(_t);
    m->setText(_m);
    m->exec();
    delete m;
}
