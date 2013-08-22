#ifndef DIALOG_H
#define DIALOG_H

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>


#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

#define __DEBUG
extern QTextStream *pDS;
#define DS *pDS


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    bool rdDomains(QFile& fDomains);
    bool rdCommands(QFile& fCommands);
    void idleTime(int cnt);
    
private:
    void command(const QString& cmd);
    QStringList         domains;
    QList<QStringList>  servers;
    QString             rdpcmd;
    QString             offcmd;
    QStringList         goNames;
    QStringList         goCommands;
    Ui::Dialog *ui;
private slots:
    void    go();
    void    logOn();
    void    changed(QString);
    void    setdomain(int ix);
};

// 5 perc
#define IDLETIME 600
class QMyApplication : public QApplication
{
    Q_OBJECT
public:
    QMyApplication(int argc, char *argv[]);
protected:
    bool notify ( QObject * receiver, QEvent * event );
public:
    QTimer  m_timer;
    int idleCounter;
    Dialog *pw;
public slots:
    void oneSec();
};

#endif // DIALOG_H
