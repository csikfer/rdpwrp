#include "control.h"
#include "parser.h"
#include "dialog.h"

quint16     cmdPort = 22022;

int cCntrl::cmdRet;
int cCntrl::cmdTo = 5000;       // Command Time Out 5 sec
cCntrl * cCntrl::pItem = NULL;


cCntrl::cCntrl()
{
    pSock = new QUdpSocket(this);
    if (!pSock->bind(localAddr, cmdPort)) {
        QString msg = QObject::trUtf8("Hiba a UDPSocket bint(%1, %2) hívásban : %3")
                .arg(localAddr.toString())
                .arg(cmdPort)
                .arg(pSock->errorString());
        critical(msg);
    }
    pItem = this;
    connect(pSock, SIGNAL(readyRead()), this, SLOT(execCtrlRCmd()));
}

cCntrl::~cCntrl()
{
    ;
}

void    cCntrl::execCtrlRCmd()
{
    while (pSock->hasPendingDatagrams()) {
        QHostAddress    remoteHost;
        quint16         remotePort;
        QByteArray b;
        int siz = pSock->pendingDatagramSize();
        b.resize(siz);
        if (siz != pSock->readDatagram(b.data(), siz, &remoteHost, &remotePort)) {
            pSock->writeDatagram(QByteArray("E:READ\n"), remoteHost, remotePort);
            continue;
        }
        QByteArray out;
        int i = parseCommand(b, out);
        out.prepend(QString("R:%1\n").arg(i).toUtf8());
        if (out.size() > CTRLMAXPACKETSIZE) {
            out.resize(CTRLMAXPACKETSIZE -1);
            out.append('\\');
        }
        pSock->writeDatagram(out, remoteHost, remotePort);
    }
}

void cCntrl::_command(const QString& _cmd)
{
    cmdRet = -1;
    QProcess    proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.execute(_cmd);
    if (proc.waitForFinished(cmdTo)) {
        QByteArray o = proc.readAll();
        yyprint(o);
        cmdRet = proc.exitCode();
    }
}

void cCntrl::getRun()
{
    const QProcess *p = mainDialog::getProcess();
    if (p == NULL) yyprint("NULL\n");
    else {
        QString o;
        switch (p->state()) {
        case QProcess::NotRunning:  o = "NOT RUNNING\n";    break;
        case QProcess::Starting:    o = "STARTING\n";       break;
        case QProcess::Running:     o = "RUNNING\n";        break;
        default:                    o = "UNKNOWN\n";        break;
        }
        o += p->program() + "\n";
        yyprint(o);
    }
    cmdRet = 0;
}
