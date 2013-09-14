#ifndef DIALOG_H
#define DIALOG_H

#include "main.h"
#include "idletimeout.h"
#include <QStringList>
#include <QComboBox>

namespace Ui {
class Dialog;
}

typedef QPair<QString, QString> QStringPair;
typedef QList<QStringPair>      QStringPairList;

class mainDialog : public QWidget
{
    friend void message(const QString& _t, const QString& _m);
    Q_OBJECT
public:
    explicit mainDialog(QWidget *parent = 0);
    ~mainDialog();
    void    set();
    bool    runing() { return !(pProc == NULL || pProc->state() == QProcess::NotRunning); }
    void    stopProc();
protected:
    void timerEvent(QTimerEvent *pTe);
    /// Egy parancs végrahajtása
    void command(const QString& cmd, int minTm = minProgTime);
    Ui::Dialog *ui;
    /// Domain nevek listája
    QStringList         domains;
    /// Sterver listák a domain-ekhez
    QList<QStringList>  servers;
    /// Alternatív RDP program a sterverekhez (alapértelmezetten üres stringek)
    QList<QStringList>  rdpCmds;
    /// Az alapértelmezett RDP program
    QString             rdpcmd;
    /// Gép kikapcsolása parancs
    QString             offcmd;
    /// Gép újraindítása parancs
    QString             rescmd;
    /// Help megjelenítése parancs
    QString             hlpcmd;
    /// Browser parancs(ok)
    QStringPairList     browsercmd;
    /// Plussz gombok, a megjelenített név
    QStringList         goNames;
    /// Plussz gombok, a parancs
    QStringList         goCommands;
    /// Plussz gombok, ikonok
    QStringList         goIcons;
    /// Plussz gombok, min futási idő
    QList<int>          goTimes;
    QButtonGroup      * pButtonGroup;
    QComboBox         * pSelBrowser;
    QProcess          * pProc;
    QString             procOut;
    QString             lastCommand;
    int                 procTime;
    int                 timerId;
    static mainDialog *     pItem;
    int                 actMinProgTime;
    cIdleTimeOut *      pToBox;
    QString             masterUser;
    QString             masterPsw;
    bool                procesStop;
private slots:
    void    logOn();                ///< KogOn gombot megnyomta
    void    browser();              ///< Browser idítása gomb
    void    off();                  ///< kikapcs gomb
    void    reboot();               ///< ewboor gomb
    void    restart();              ///< reboot gomb
    void    help();                 ///< segítség gomb
    void    go(int id);             ///< Valamelyik konfigban megadott parancs gombja
    void    chgUsrOrPw(QString);    ///< Az user, vagy jelszó mező megváltozott
    void    selDomain(int ix);      ///< kiválasztott egy domaint
    void    enter();                ///< Megnyomta az Enter billentyűt

    void    procStarted();
    void    procReadyRead();
    void    procError(QProcess::ProcessError e);
    void    procFinished(int r);
    /// Kikapcsolásra figyelmeztetés
    void    idleTimeOutBox();
signals:
    void    doTOBox();
public:
#define _ADD(m, p)  pItem->m << *p;  delete p
#define _SET(m, p)  pItem->m =  *p;  delete p
#define _GET(m)     return pItem->m
    static void addDomain(QStringPair * pDom, QStringPairList *pServers);
    static void addCommand(QString *pName, QString *pCmd, QString *pIcon, int _to = -1) {
        _ADD(goNames, pName);  _ADD(goCommands, pCmd); _ADD(goIcons, pIcon);
        pItem->goTimes << ((_to < 0) ? minProgTime : _to);
    }
    static void setRdpCmd(QString * pCmd)   { _SET(rdpcmd, pCmd); }
    static void setOffCmd(QString * pCmd)   { _SET(offcmd, pCmd); }
    static void setResCmd(QString * pCmd)   { _SET(rescmd, pCmd); }
    static void setHelpCmd(QString * pCmd)  { _SET(hlpcmd, pCmd); }
    static bool setBrowserCmd(QString * pCmd);
    static bool setBrowserCmds(QStringPairList * pCmds);
    static void setMaster(QString * pU, QString * pP) { _SET(masterUser, pU); _SET(masterPsw, pP); }

    static const QString& getRdpCmd()       { _GET(rdpcmd); }
    static const QString& getOffCmd()       { _GET(offcmd); }
    static const QString& getResCmd()       { _GET(rescmd); }
    static const QString& getHelpCmd()      { _GET(hlpcmd); }

    static const QProcess *getProcess()     { return pItem->pProc; }
private:
    void doExit(const QString& cmd = QString());
    inline QFrame *hLine();
    inline QPushButton *button(QString &txt, QString ico);
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
protected:
    QString     lastCmd;
public:
    static QString getLastCmd() { return pItem->lastCmd; }
#else
public:
    static QString getLastCmd() { return pItem->pProc->program(); }
public:
#endif
};

#endif // DIALOG_H
