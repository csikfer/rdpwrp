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

#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

#define FULLSCREEN 0
#define __DEBUG
extern QTextStream *pDS;
#define DS *pDS

/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol, alapértelmezett érték
#define IDLETIME        300
/// Ha nincs mozgás, akkor ennyi másodperc után kikapcsol
extern int idleTime;
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs
#define IDLEDIALOGTIME   60
/// A kikapcsolásra figyelmeztető ablak eddig aktív, aztán kikapcs, alapértelmezett érték
extern int idleDialogTime;
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús, alapértelmezett érték kezdő értéke
#define MINPRCTM         10
/// Ha a hívott program futási ideje ennél rövidebb, akkor gyanús, alapértelmezett érték
extern int minProgTime;

/// Idle time counter
extern int idleTimeCnt;
#define KIOSKIDLETIME 120
extern int kioskIdleTime;
#define KIOSKIDLEDIALOGTIME 30
extern int kioskIdleDialogTime;
extern bool kioskIsOn;

extern QString sSugg;
extern QString sCrit;
extern QString hostname;
extern int     desktopHeiht, desktopWidth;

extern bool    isDown;
extern bool    isKiosk;

extern QString             yyLastError;
extern QString             yyLastLine;
extern int                 yyLineNo;
extern QString             yyLine;
extern int parseConfig(QFile *_in);

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

#endif // MAIN_H
