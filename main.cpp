#include "dialog.h"
#include <stdio.h>
#include <QDesktopWidget>

QTextStream *pDS = NULL;
int idleTimeCnt = 0;
QString sSugg;
QString sCrit;
QString hostname;
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


