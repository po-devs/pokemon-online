#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

namespace NetworkCli
{

class Network : public QTcpSocket
{
public:
    Network(const QString &host="", quint16 port=0);
    /* Useful inherited functions: connectToHost */
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
