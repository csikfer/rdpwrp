#ifndef DIALOG_H
#define DIALOG_H

#include "main.h"
#include "idletimeout.h"
#include <QStringList>
#include <QComboBox>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

typedef QPair<QString, QString> QStringPair;
typedef QList<QStringPair>      QStringPairList;

class mainDialog : public QMainWindow
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
    Ui::MainWindow *ui;
    // Status bar:
    QLabel      *pStatusBarAutoOff;
    QLabel      *pStatusBarAutoOffSec;
    QLabel      *pStatusBarAutoOffAfter;
    QLabel      *pStatusBarRightLabel;
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
    cAppButtonList      browsers;
    /// Plussz gombok
    cAppButtonList      goList;
    /// Plussz gombok, min futási idő
    QList<int>          goTimes;
    QButtonGroup      * pButtonGroup;
    QComboBox         * pSelBrowser;
    QProcess          * pProc;
    QString             procOut;
    QString             lastCommand;
    int                 procTime;
    int                 timerId;
    static mainDialog * pItem;
    int                 actMinProgTime;
    cIdleTimeOut *      pToBox;
    QString             masterUser;
    QString             masterPsw;
    bool                procesStop;
    QString             trHuPath;
    QString             trEnPath;
    QTranslator m_translator; // contains the translations for this application
    QTranslator m_translatorQt; // contains the translations for qt
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
    void    setHu();
    void    setEn();
signals:
    void    doTOBox();
public:
#define _ADD(m, p)  pItem->m << *p;  delete p
#define _SET(m, p)  pItem->m =  *p;  delete p
#define _GET(m)     return pItem->m
    static void addDomain(QStringPair * pDom, QStringPairList *pServers);
    static void addCommand(QString *pName, QString *pCmd, QString *pIcon, int _to = -1) {
        pItem->goList << cAppButton(pIcon, pName, pCmd);
        pItem->goTimes << ((_to < 0) ? minProgTime : _to);
    }
    static void setRdpCmd(QString * pCmd)   { _SET(rdpcmd, pCmd); }
    static void setOffCmd(QString * pCmd)   { _SET(offcmd, pCmd); }
    static void setResCmd(QString * pCmd)   { _SET(rescmd, pCmd); }
    static void setHelpCmd(QString * pCmd)  { _SET(hlpcmd, pCmd); }
    static bool setBrowserCmd(QString * pCmd);
    static bool setBrowserCmds(cAppButtonList * pCmds);
    static void setMaster(QString * pU, QString * pP) { _SET(masterUser, pU); _SET(masterPsw, pP); }

    static const QString& getRdpCmd()       { _GET(rdpcmd); }
    static const QString& getOffCmd()       { _GET(offcmd); }
    static const QString& getResCmd()       { _GET(rescmd); }
    static const QString& getHelpCmd()      { _GET(hlpcmd); }

    static const QProcess *getProcess()     { return pItem->pProc; }
private:
    void doExit(const QString& cmd = QString());
    inline QFrame *hLine();
    inline QPushButton *button(const QString &_txt, const QString &ico);
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
