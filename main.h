#ifndef MAIN_H
#define MAIN_H

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
#include <QtNetwork>
#include <QIcon>

#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

/// Ha értéke nem nulla, akkor a main window teljes képernyős
/// Tesztelésnél praktikusan legyen nulla
extern bool mainIsFullScreen;
/// Ha értéke nem nulla, akkor program indításakor a fő ablakot láthatatlanná teszi
#define HIDEMAINIFRUNCHILD 1

/// Ha értéke nem nulla , akkor a debug üzenetek kiíródnak, egyébként a /dev/null -ba mennek.
#define __DEBUG 1
extern QTextStream *pDS;
/// debug stream
#define DS *pDS

/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol, alapértelmezett érték
#define IDLETIME        300
/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol
extern int idleTime;
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs/alaphelyzet
#define IDLEDIALOGTIME   60
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs/alaphelyzet, alapértelmezett érték
extern int idleDialogTime;
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús, alapértelmezett érték kezdő értéke
#define MINPRCTM         5
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús, alapértelmezett érték
extern int minProgTime;

/// Idle time counter
extern int idleTimeCnt;

extern QString sSugg;
extern QString sCrit;
extern QString sWarn;

extern QString hostname;
extern QString localAddrStr;
extern QHostAddress    localAddr;

extern int     desktopHeight, desktopWidth;

extern bool    isDown;      ///< Állapot jelző
extern bool    isKiosk;     ///< Mód
extern enum eActLang { AL_HU, AL_EN } actLang;

/* // Az idletime() ezt kiváltja
class QMyApplication : public QApplication
{
    Q_OBJECT
public:
    QMyApplication(int argc, char *argv[]);
protected:
    bool notify ( QObject * receiver, QEvent * event );
};*/

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

extern void message(const QString& _t, const QString& _m);

extern int getIdleTime(int _min = 0);

inline void critical(QString msg)
{
    QMessageBox::critical(NULL, sCrit, msg + sSugg);
    ::exit(1);
}

class cAppButton {
public:
    QString mIcon;
    QString mText;
    QString mCmd;
    cAppButton() : mIcon(), mText(), mCmd() {;}
    cAppButton(QString *pI, QString *pT, QString *pC) : mIcon(*pI), mText(*pT), mCmd(*pC) {
        delete pI;  delete pT;  delete pC;
    }
    QIcon   icon() { return QIcon(mIcon); }
};

typedef QList<cAppButton> cAppButtonList;


#endif // MAIN_H
