#include "dialog.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Dialog w;

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
    if (!w.rdDomains(fDomains)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A tartomány lista fájl nem értelmezhetó.") + e);
        exit(1);
    }
    fDomains.close();

    QFile   fCommands(home.filePath("commands"));
    if (!fCommands.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(NULL, t, QObject::trUtf8("A parancs lista fájl nem olvasható, vagy nem létezik.") + e);
        exit(1);
    }
    if (!w.rdCommands(fCommands)) {
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
            w.setStyleSheet(s);
        }
        fStyles.close();
    }

    w.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    w.showFullScreen();
    
    return a.exec();
}
