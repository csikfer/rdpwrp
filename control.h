#ifndef CONTROL_H
#define CONTROL_H

#include <main.h>
#include <QtNetwork>

#define CTRLMAXPACKETSIZE 1500

extern  quint16     cmdPort;

class cCntrl : public QObject {
    Q_OBJECT
public:
    cCntrl();
    ~cCntrl();
private:
    QUdpSocket     *pSock;
    static cCntrl  *pItem;
    static int      cmdRet;
    static int      cmdTo;
private slots:
    void    execCtrlRCmd();
public:
    static void _command(const QString& _cmd);
    static void command(QString * _cmd) { _command(*_cmd); delete _cmd; }
    static void _ok()                   { cmdRet = 0;   DS << __PRETTY_FUNCTION__ << endl; }
    static void setCmdTo(int _to)       { cmdTo = _to; }
    static void getRun();
};

#endif // CONTROL_H
