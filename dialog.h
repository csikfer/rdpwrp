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
    static QStringList         domains;
    /// Sterver listák a domain-ekhez
    static QList<QStringList>  servers;
    /// Alternatív RDP program a sterverekhez (alapértelmezetten üres stringek)
    static QList<QStringList>  rdpCmds;
    /// Az alapértelmezett RDP program
    static QString             rdpcmd;
    /// Gép kikapcsolása parancs
    static QString             offcmd;
    /// Gép újraindítása parancs
    static QString             rescmd;
    /// Help megjelenítése parancs
    static QString             hlpcmd;
    /// Browser parancs(ok)
    static cAppButtonList      browsers;
    /// Plussz gombok
    static cAppButtonList      goList;
    /// Plussz gombok, min futási idő
    static QList<int>          goTimes;
    QButtonGroup      * pButtonGroup;
    QProcess          * pProc;
    QString             procOut;
    QString             lastCommand;
    int                 procTime;
    int                 timerId;
    static mainDialog * pItem;
    int                 actMinProgTime;
    cIdleTimeOut *      pToBox;
    static QString             masterUser;
    static QString             masterPsw;
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
    static void addDomain(QStringPair * pDom, QStringPairList *pServers);
    static void addCommand(QString *pName, QString *pCmd, QString *pIcon, int _to = -1) {
        goList << cAppButton(pIcon, pName, pCmd);
        goTimes << ((_to < 0) ? minProgTime : _to);
    }
    static void setRdpCmd(QString * pCmd)   { rdpcmd = *pCmd; delete pCmd; }
    static void setOffCmd(QString * pCmd)   { offcmd = *pCmd; delete pCmd; }
    static void setResCmd(QString * pCmd)   { rescmd = *pCmd; delete pCmd; }
    static void setHelpCmd(QString * pCmd)  { hlpcmd = *pCmd; delete pCmd; }
    static bool setBrowserCmd(QString * pCmd);
    static bool setBrowserCmds(cAppButtonList * pCmds);
    static void setMaster(QString * pU, QString * pP) { masterUser = *pU; masterPsw = *pP;  delete pU; delete pP; }

    static const QString& getRdpCmd()       { return rdpcmd; }
    static const QString& getOffCmd()       { return offcmd; }
    static const QString& getResCmd()       { return rescmd; }
    static const QString& getHelpCmd()      { return hlpcmd; }

    static const QProcess *getProcess()     { return pItem == NULL ? NULL : pItem->pProc; }
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
