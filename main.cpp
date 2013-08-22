#include "dialog.h"
#include <stdio.h>

QTextStream *pDS = NULL;

QMyApplication::QMyApplication(int argc, char *argv[]) : QApplication(argc, argv), m_timer() {
    pw = NULL;
    idleCounter = IDLETIME;
    m_timer.setInterval(1000);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(oneSec()));
    m_timer.start();
}

bool QMyApplication::notify ( QObject * receiver, QEvent * event )
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::KeyPress) {
        idleCounter = IDLETIME;
    }
    return QApplication::notify(receiver, event);
}

void QMyApplication::oneSec()
{
    if (pw != NULL) {
        pw->idleTime(idleCounter--);
    }
}

int main(int argc, char *argv[])
{
    QMyApplication a(argc, argv);

#ifdef __DEBUG
    pDS = new QTextStream(stderr, QIODevice::WriteOnly);
#else
    pDS = new QTextStream(fopen("/dev/null", "w"), QIODevice::WriteOnly);
#endif

    a.pw = new Dialog();

    QDir    home(QString("./.%1").arg(STR(APPNAME)));

    QString e = QObject::trUtf8(" Forduljon a rendszergazdához!");
    QString t = QObject::trUtf8("Végzetes hiba!");

    if (!home.isReadable()) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A konfigurációs mappa nem olvasható, vagy nem létezik.") + e);
        exit(1);
    }
    QFile   fDomains(home.filePath("domains"));
    if (!fDomains.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A tartomány lista fájl nem olvasható, vagy nem létezik.") + e);
        exit(1);
    }
    if (!a.pw->rdDomains(fDomains)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A tartomány lista fájl nem értelmezhetó.") + e);
        exit(1);
    }
    fDomains.close();

    QFile   fCommands(home.filePath("commands"));
    if (!fCommands.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A parancs lista fájl nem olvasható, vagy nem létezik.") + e);
        exit(1);
    }
    if (!a.pw->rdCommands(fCommands)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A parancs lista fájl nem értelmezhetó.") + e);
        exit(1);
    }
    fCommands.close();

    QFile   fStyles(home.filePath("styles"));
    if (fStyles.open(QIODevice::ReadOnly)) {
        QByteArray l;
        while (!(l = fStyles.readLine()).isEmpty()) {
            QString s = QString::fromUtf8(l).simplified();
            if (s.isEmpty()) continue;
            a.pw->setStyleSheet(s);
        }
        fStyles.close();
    }

    a.pw->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    a.pw->showFullScreen();
    
    bool r = a.exec();
    delete a.pw;
    return r;
}
