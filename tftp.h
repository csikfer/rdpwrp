#ifndef TFTP_H
#define TFTP_H

// Forrás: http://lau.engineering.uky.edu/2013/08/17/qt-based-tftp-class/

#include <QtNetwork>

#define TFTPMAXPACKETSIZE 512
#define TFTPUDPPORT    69

extern enum QAbstractSocket::NetworkLayerProtocol ipProto;

class QTFtpClient {
public:
    QTFtpClient(QString remoteAddressString, int port = TFTPUDPPORT);
    ~QTFtpClient();

    bool getByteArray(QString filename, QByteArray *requestedFile);
    // bool putByteArray(QString filename, QByteArray transmittingFile);

    QString lastError() { return lastErrorString; }
    bool operator !()   { return !lastErrorString.isEmpty(); }

private:
    bool bindSocket();
    bool _getByteArray(QString filename, QByteArray *requestedFile);
    QByteArray getFilePacket(QString filename);
    // QByteArray putFilePacket(QString filename);

    QString lastErrorString;
    QUdpSocket *socket;
    QHostAddress    remoteAddr;
    quint16         ctrlPort;
    quint16         dataPort;
};

#endif // TFTP_H
