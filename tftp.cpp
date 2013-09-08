#include <QNetworkInterface>
#include "tftp.h"
#include "main.h"   // csak a debugg üzenetek miatt

// #define TFTPMODE "octet"
#define TFTPMODE "netascii"

#if     __DEBUG
static QString dump(QByteArray& b)
{
    QString r = QChar('"');
    for (int i = 0; i < b.size(); ++i) {
        unsigned char c = (unsigned char)b.at(i);
        r += isprint(c) ? QChar((int)c) : ("\\" + QString::number(c, 8));
    }
    return r + QChar('"');
}
#else //__DEBUG
static QString dump(QByteArray&)
{
    return QString();
}
#endif//__DEBUG

#define WAITTO          2000
/// igazándiból keresni kéne egy szabad portot, de hogyan ?
#define LOCALUDPPORT    12345

enum QAbstractSocket::NetworkLayerProtocol ipProto = QAbstractSocket::UnknownNetworkLayerProtocol;

QTFtpClient::QTFtpClient(QString remoteAddressString, int port)
    : lastErrorString(), remoteAddr()
{
    socket = NULL;
    ctrlPort = port;
    dataPort = -1;
    remoteAddr = QHostAddress(remoteAddressString);
    if (remoteAddr.isNull()) {
        lastErrorString = QObject::trUtf8("Nem értelmezhető tftp szerver cím : %1").arg(remoteAddressString);
        return;
    }
}

QTFtpClient::~QTFtpClient()
{
    if (socket) delete socket;
}

bool QTFtpClient::bindSocket()
{
    // SEE IF WE ALREADY HAVE A BOUND SOCKET
    // AND IF SO, WE NEED TO DELETE IT
    if (socket != NULL) {
        delete socket;
    }

    // CREATE A NEW SOCKET
    socket=new QUdpSocket();

    // AND SEE IF WE CAN BIND IT TO A LOCAL IP ADDRESS AND PORT
    DS << "bind " << localAddr.toString() << ", " << LOCALUDPPORT << endl;
    return socket->bind(localAddr, LOCALUDPPORT);
}

/*
bool QTFtpClient::putByteArray(QString filename, QByteArray transmittingFile)
{
    // BIND OUR LOCAL SOCKET TO AN IP ADDRESS AND PORT
    if (!bindSocket()) {
        lastErrorString = QObject::trUtf8("Hiba a bintSocket() hívásban : %1").arg(socket->errorString());
        return(false);
    }

    // CREATE REQUEST PACKET AND SEND TO HOST
    // WAIT UNTIL MESSAGE HAS BEEN SENT, QUIT IF TIMEOUT IS REACHED
    QByteArray reqPacket=putFilePacket(filename);
    DS << "Send : " << dump(reqPacket) << endl;
    if (socket->writeDatagram(reqPacket, remoteAddr, port) != reqPacket.length()){
        lastErrorString = QObject::trUtf8("did not send packet to host :(  %1").arg(socket->errorString());
        return(false);
    }

    // CREATE PACKET COUNTERS TO KEEP TRACK OF MESSAGES
    unsigned short incomingPacketNumber=0;
    unsigned short outgoingPacketNumber=0;

    // NOW WAIT HERE FOR INCOMING DATA
    bool messageCompleteFlag=false;
    while (1){
        // WAIT FOR AN INCOMING PACKET
        if (socket->hasPendingDatagrams() || socket->waitForReadyRead(WAITTO)){
            // ITERATE HERE AS LONG AS THERE IS ATLEAST A
            // PACKET HEADER'S WORTH OF DATA TO READ
            QByteArray incomingDatagram;
            incomingDatagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(incomingDatagram.data(), incomingDatagram.length());

            // MAKE SURE FIRST BYTE IS 0
            char *buffer=incomingDatagram.data();
            char zeroByte=buffer[0];
            if (zeroByte != 0x00) {
                lastErrorString = QObject::trUtf8("Incoming packet has invalid first byte (%1).").arg((int)zeroByte);
                return(false);
            }

            // READ UNSIGNED SHORT PACKET NUMBER USING LITTLE ENDIAN FORMAT
            // FOR THE INCOMING UNSIGNED SHORT VALUE BUT BIG ENDIAN FOR THE
            // INCOMING DATA PACKET
            unsigned short incomingMessageCounter;
            *((char*)&incomingMessageCounter+1)=buffer[2];
            *((char*)&incomingMessageCounter+0)=buffer[3];

            // CHECK INCOMING MESSAGE ID NUMBER AND MAKE SURE IT MATCHES
            // WHAT WE ARE EXPECTING, OTHERWISE WE'VE LOST OR GAINED A PACKET
            if (incomingMessageCounter == incomingPacketNumber){
                incomingPacketNumber++;
            } else {
                lastErrorString = QObject::trUtf8("error on incoming packet number %1 vs expected %2").arg(incomingMessageCounter).arg(incomingPacketNumber);
                return(false);
            }

            // CHECK THE OPCODE FOR ANY ERROR CONDITIONS
            char opCode=buffer[1];
            if (opCode != 0x04) { // ack packet should have code 4 and should be ack+1 the packet we just sent
                lastErrorString = QObject::trUtf8("Incoming packet returned invalid operation code (%1).").arg((int)opCode);
                return(false);
            } else {
                // SEE IF WE NEED TO SEND ANYMORE DATA PACKETS BY CHECKING END OF MESSAGE FLAG
                if (messageCompleteFlag) break;

                // SEND NEXT DATA PACKET TO HOST
                QByteArray transmitByteArray;
                transmitByteArray.append((char)0x00);
                transmitByteArray.append((char)0x03); // send data opcode
                transmitByteArray.append(*((char*)&outgoingPacketNumber+1));
                transmitByteArray.append(*((char*)&outgoingPacketNumber));

                // APPEND DATA THAT WE WANT TO SEND
                int numBytesAlreadySent=outgoingPacketNumber*MAXPACKETSIZE;
                int bytesLeftToSend=transmittingFile.length()-numBytesAlreadySent;
                if (bytesLeftToSend < MAXPACKETSIZE){
                    messageCompleteFlag=true;
                    if (bytesLeftToSend > 0){
                        transmitByteArray.append((transmittingFile.data()+numBytesAlreadySent), bytesLeftToSend);
                    }
                } else {
                    transmitByteArray.append((transmittingFile.data()+numBytesAlreadySent), MAXPACKETSIZE);
                }

                // SEND THE PACKET AND MAKE SURE IT GETS SENT
                if (socket->writeDatagram(transmitByteArray, remoteAddr, port) != transmitByteArray.length()){
                    lastErrorString = QObject::trUtf8("did not send data packet to host :(  %1").arg(socket->errorString());
                    return(false);
                }

                // NOW THAT WE'VE SENT AN ACK SIGNAL, INCREMENT SENT MESSAGE COUNTER
                outgoingPacketNumber++;
            }
        } else {
            lastErrorString = QObject::trUtf8("No message received from host :(  %1").arg(socket->errorString());
            return(false);
        }
    }
    lastErrorString.clear();
    return(true);

}
*/
bool QTFtpClient::getByteArray(QString filename, QByteArray *requestedFile)
{
    DS << __PRETTY_FUNCTION__ << endl;
    // BIND OUR LOCAL SOCKET TO AN IP ADDRESS AND PORT
    if (!bindSocket()) {
        lastErrorString = QObject::trUtf8("Hiba a bindSocket() hívásban : %1").arg(socket->errorString());
        return(false);
    }

    // CLEAN OUT ANY INCOMING PACKETS
    while (socket->hasPendingDatagrams()){
        QByteArray byteArray;
        byteArray.resize(socket->pendingDatagramSize());
        socket->readDatagram(byteArray.data(), byteArray.length());
    }

    // CREATE REQUEST PACKET AND SEND TO HOST
    // WAIT UNTIL MESSAGE HAS BEEN SENT, QUIT IF TIMEOUT IS REACHED
    QByteArray reqPacket=getFilePacket(filename);
    DS << "Send : " << dump(reqPacket) << " to " << remoteAddr.toString() << ":" << ctrlPort << endl;
    if (socket->writeDatagram(reqPacket, remoteAddr, ctrlPort) != reqPacket.length()){
        lastErrorString =  QObject::trUtf8("did not send packet to host :(  %1").arg(socket->errorString());
        return(false);
    }

    // CREATE PACKET COUNTERS TO KEEP TRACK OF MESSAGES
    unsigned short incomingPacketNumber=1;
    unsigned short outgoingPacketNumber=1;

    // NOW WAIT HERE FOR INCOMING DATA
    bool messageCompleteFlag=false;
    while (!messageCompleteFlag){
        // WAIT FOR AN INCOMING PACKET
        if (socket->hasPendingDatagrams() || socket->waitForReadyRead(WAITTO)){
            // ITERATE HERE AS LONG AS THERE IS ATLEAST A
            // PACKET HEADER'S WORTH OF DATA TO READ
            QByteArray incomingDatagram;
            incomingDatagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(incomingDatagram.data(), incomingDatagram.length(), NULL, &dataPort);
            DS << "Recived #" << incomingPacketNumber << " (size " << incomingDatagram.length() << " ):\n" << dump(incomingDatagram) << endl;

            // MAKE SURE FIRST BYTE IS 0
            char *buffer=incomingDatagram.data();
            char zeroByte=buffer[0];
            if (zeroByte != 0x00) {
                lastErrorString = QObject::trUtf8("Incoming packet has invalid first byte (%1).").arg((int)zeroByte);
                return(false);
            }

            // READ UNSIGNED SHORT PACKET NUMBER USING LITTLE ENDIAN FORMAT
            // FOR THE INCOMING UNSIGNED SHORT VALUE BUT BIG ENDIAN FOR THE
            // INCOMING DATA PACKET
            unsigned short incomingMessageCounter;
            *((char*)&incomingMessageCounter+1)=buffer[2];
            *((char*)&incomingMessageCounter+0)=buffer[3];

            // CHECK INCOMING MESSAGE ID NUMBER AND MAKE SURE IT MATCHES
            // WHAT WE ARE EXPECTING, OTHERWISE WE'VE LOST OR GAINED A PACKET
            if (incomingMessageCounter == incomingPacketNumber) {
                // COPY THE INCOMING FILE DATA
                QByteArray incomingByteArray(&buffer[4], incomingDatagram.length()-4);

                // SEE IF WE RECEIVED A COMPLETE 512 BYTES AND IF SO,
                // THEN THERE IS MORE INFORMATION ON THE WAY
                // OTHERWISE, WE'VE REACHED THE END OF THE RECEIVING FILE
                if (incomingByteArray.length() < TFTPMAXPACKETSIZE){
                    messageCompleteFlag=true;
                }

                // APPEND THE INCOMING DATA TO OUR COMPLETE FILE
                requestedFile->append(incomingByteArray);

                incomingPacketNumber++;
            }
            else if (incomingMessageCounter > incomingPacketNumber) {
                lastErrorString = QObject::trUtf8("error on incoming packet number %1 vs expected %2").arg(incomingMessageCounter).arg(incomingPacketNumber);
                return(false);
            }

            // CHECK THE OPCODE FOR ANY ERROR CONDITIONS
            char opCode=buffer[1];
            if (opCode != 0x03) { /* ack packet should have code 3 (data) and should be ack+1 the packet we just sent */
                lastErrorString = QObject::trUtf8("Incoming packet returned invalid operation code (%1).").arg((int)opCode);
                return(false);
            } else {
                // SEND PACKET ACKNOWLEDGEMENT BACK TO HOST REFLECTING THE INCOMING PACKET NUMBER
                QByteArray ackByteArray;
                ackByteArray.append((char)0x00);
                ackByteArray.append((char)0x04);
                ackByteArray.append(*((char*)&incomingMessageCounter+1));
                ackByteArray.append(*((char*)&incomingMessageCounter));

                DS << "Send : " << dump(ackByteArray) << " to " << remoteAddr.toString() << ":" << dataPort << endl;
                // SEND THE PACKET AND MAKE SURE IT GETS SENT
                if (socket->writeDatagram(ackByteArray, remoteAddr, dataPort) != ackByteArray.length()){
                    lastErrorString = QObject::trUtf8("did not send ack packet to host :(  %1").arg(socket->errorString());
                    return(false);
                }

                // NOW THAT WE'VE SENT AN ACK SIGNAL, INCREMENT SENT MESSAGE COUNTER
                outgoingPacketNumber++;
            }
        } else {
            lastErrorString = QObject::trUtf8("No message received from host :(  %1").arg(socket->errorString());
            return(false);
        }
    }
    lastErrorString.clear();
    return(true);
}

QByteArray QTFtpClient::getFilePacket(QString filename)
{
    QByteArray byteArray(filename.toLatin1());
    byteArray.prepend((char)0x01); // OPCODE
    byteArray.prepend((char)0x00);
    byteArray.append((char)0x00);
    byteArray.append(TFTPMODE); // MODE
    byteArray.append((char)0x00);

    return(byteArray);
}

/*
QByteArray QTFtpClient::putFilePacket(QString filename)
{
    QByteArray byteArray((char)0x00);
    byteArray.append((char)0x02); // OPCODE
    byteArray.append(filename.toLatin1());
    byteArray.append((char)0x00);
    byteArray.append(TFTPMODE); // MODE
    byteArray.append((char)0x00);

    return(byteArray);
}
*/
