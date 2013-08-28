#ifndef DIALOG_H
#define DIALOG_H

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLayout>
#include <QTextStream>
#include <QTimer>
#include <QProcess>
#include <QMessageBox>
#include <QButtonGroup>


#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

#define __DEBUG
extern QTextStream *pDS;
#define DS *pDS

/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol
#define IDLETIME        300
extern int idleTime;
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs
#define IDLEDIALOGTIME   60
extern int idleDialogTime;
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús
#define MINPRCTM         10
extern int minProgTime;

/// Idle time counter
extern int idleTimeCnt;

extern QString sSugg;
extern QString sCrit;
extern QString hostname;
extern int     desktopHeiht, desktopWidth;

extern QString             yyLastError;
extern QString             yyLastLine;
extern int                 yyLineNo;
extern QString             yyLine;
int parseConfig(QFile *_in);

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void    set();
protected:
    void timerEvent(QTimerEvent *pTe);
    /// Egy parancs végrahajtása
    void command(const QString& cmd, int minTm = minProgTime);
    /// Kikapcsolásra figyelmeztetés
    void idleTimeOut();
    void message(const QString& _t, const QString& _m);
    QIcon   goIcon(int _i);
    Ui::Dialog *ui;
    QStringList         domains;
    QList<QStringList>  servers;
    QString             rdpcmd;
    QString             offcmd;
    QString             hlpcmd;
    QStringList         goNames;
    QStringList         goCommands;
    QStringList         goIcons;
    QList<int>          goTimes;
    QButtonGroup      * pButtonGroup;
    QProcess          * pProc;
    QString             procOut;
    QString             lastCommand;
    int                 procTime;
    int                 timerId;
    static Dialog *     pItem;
    int                 actMinProgTime;
private slots:
    void    logOn();
    void    off();
    void    help();
    void    go(int id);
    void    chgUsrOrPw(QString);
    void    selDomain(int ix);

    void    procStated();
    void    procReadyRead();
    void    procError(QProcess::ProcessError e);
    void    procFinished(int r);
public:
#define _ADD(m, p)  pItem->m << *p;  delete p
#define _SET(m, p)  pItem->m =  *p;  delete p
#define _GET(m)     return pItem->m
    static void addDomain(QString * pDom, QStringList *pServers) {
        _ADD(domains, pDom);    _ADD(servers, pServers);
    }
    static void addCommand(QString *pName, QString *pCmd, QString *pIcon, int _to = -1) {
        _ADD(goNames, pName);  _ADD(goCommands, pCmd); _ADD(goIcons, pIcon);
        pItem->goTimes << ((_to < 0) ? minProgTime : _to);
    }
    static void setRdpCmd(QString * pCmd)   { _SET(rdpcmd, pCmd); }
    static void setOffCmd(QString * pCmd)   { _SET(offcmd, pCmd); }
    static void setHelpCmd(QString * pCmd)  { _SET(hlpcmd, pCmd); }

    static const QString& getRdpCmd()       { _GET(rdpcmd); }
    static const QString& getOffCmd()       { _GET(offcmd); }
    static const QString& getHelpCmd()      { _GET(hlpcmd); }
};

class msgBox : public QDialog
{
public:
    msgBox(QWidget *p);
    void setText(const QString& _t) { text->setText(_t); }
protected:
    void    timerEvent(QTimerEvent *);
private:
    QVBoxLayout      *layout;
    QTextEdit        *text;
    QDialogButtonBox *buttons;
    int               timer;
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
