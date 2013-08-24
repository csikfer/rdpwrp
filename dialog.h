#ifndef DIALOG_H
#define DIALOG_H

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QProcess>


#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

#define __DEBUG
extern QTextStream *pDS;
#define DS *pDS

/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol
#define IDLETIME        66
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs
#define IDLEDIALOGTIME   60
/// Idle time counter
extern int idleTime;
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús
#define MINPRCTM         10

extern QString sSugg;
extern QString sCrit;
extern QString hostname;

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    bool rdDomains(QFile& fDomains);
    bool rdCommands(QFile& fCommands);
    
private:
    /// Egy parancs végrahajtása
    void command(const QString& cmd);
    /// Kikapcsolásra figyelmeztetés
    void idleTimeOut();
    Ui::Dialog *ui;
    QStringList         domains;
    QList<QStringList>  servers;
    QString             rdpcmd;
    QString             offcmd;
    QStringList         goNames;
    QStringList         goCommands;
    QProcess          * pProc;
    QString             procOut;
    QString             lastCommand;
    int                 procTime;
    int                 timerId;
protected:
    void timerEvent(QTimerEvent *pTe);
private slots:
    void    go();
    void    logOn();
    void    chgUsrOrPw(QString);
    void    selDdomain(int ix);

    void    procStated();
    void    procReadyRead();
    void    procError(QProcess::ProcessError e);
    void    procFinished(int r);
};

class QMyApplication : public QApplication
{
    Q_OBJECT
public:
    QMyApplication(int argc, char *argv[]);
protected:
    bool notify ( QObject * receiver, QEvent * event );
};


#endif // DIALOG_H
