#ifndef DIALOG_H
#define DIALOG_H

#include "main.h"

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    friend void message(const QString& _t, const QString& _m);
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
    void idleTimeOutBox();
    Ui::Dialog *ui;
    QStringList         domains;
    QList<QStringList>  servers;
    QString             rdpcmd;
    QString             offcmd;
    QString             rescmd;
    QString             hlpcmd;
    QString             kioskcmd;
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
    void    kiosk();
    void    off();
    void    help();
    void    go(int id);
    void    chgUsrOrPw(QString);
    void    selDomain(int ix);

    void    procStarted();
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
    static void setResCmd(QString * pCmd)   { _SET(rescmd, pCmd); }
    static void setHelpCmd(QString * pCmd)  { _SET(hlpcmd, pCmd); }
    static void setKioskCmd(QString * pCmd) { _SET(kioskcmd, pCmd); isKiosk = true; }

    static const QString& getRdpCmd()       { _GET(rdpcmd); }
    static const QString& getOffCmd()       { _GET(offcmd); }
    static const QString& getResCmd()       { _GET(rescmd); }
    static const QString& getHelpCmd()      { _GET(hlpcmd); }
private:
    inline QFrame *hLine();
    inline QPushButton *button(QString txt, QString ico);
};

#endif // DIALOG_H
