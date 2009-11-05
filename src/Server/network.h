#ifndef NETWORK_H
#define NETWORK_H


#include <QtNetwork>

class Network : public QTcpSocket
{
    Q_OBJECT
public:
    Network(const QString &host="", quint16 port=0);
    /* Useful inherited functions:
	   connectToHost(QString&, quint16): connects to host & port
	   error: returns an enum (QAbstractSocket::SocketError) describing the error
	   errorString: returns a readable string describing the error
	   isValid: returns whether the socket is valid or not!
	   close: closes the socket after writing the pending data
	   abort: abruptly closes the socket */
public slots:
    void onReceipt();
    void send(const QByteArray &message);
signals:
    void isFull(QByteArray command);
    /* Useful inherited signals: error, disconnected */
private:
    /* internal variables for the protocol */
    bool commandStarted;
    quint16 remainingLength;
};

#endif // NETWORK_H
