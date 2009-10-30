#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

namespace NetworkCli
{

class Network : public QTcpSocket
{
    Q_OBJECT
public:
    Network(const QString &host="", quint16 port=0);
    /* Useful inherited functions:
           connectToHost(QString&, quint16): connects to host & port
           error: returns an enum (QAbstractSocket::SocketError) describing the error
           errorString: returns a readable string describing the error
           isValid: returns whether the socket is valid or not! */
public slots:
    void onReceipt();
signals:
    void isFull(QByteArray command);
    /* Useful inherited signals: error, disconnected */
private:

    /* internal variables for the protocol */
    bool commandStarted;
    quint16 remainingLength;
};

}

#endif // NETWORK_H
