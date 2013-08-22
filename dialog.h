#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDir>
#include <QMessageBox>

#define APPNAME rdpwrp

#define _STR(s) #s
#define STR(s)  _STR(s)

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
    
private:
    void command(const QString& cmd);
    QStringList         domains;
    QList<QStringList>  servers;
    QString             rdpcmd;
    QStringList         goNames;
    QStringList         goCommands;
    Ui::Dialog *ui;
private slots:
    void    go();
    void    logOn();
    void    changed(QString);
    void    setdomain(int ix);
};

#endif // DIALOG_H
