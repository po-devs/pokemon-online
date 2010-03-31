#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

/*
    This is what is used to communicate with the outside.

    You send data with send(), and the signal isFull() warns you when a message arrived.



    Here are the details you don't need to know:

    Actually the protocol is here: the first two bytes represent the length of the message (in a 256 numerical base), and then
    there's the message.

    The first byte is therefore length() / 256, and the second length() % 256.

    The drawback to that mini-protocol is that you cannot send data longer than (256*256 - 1), but it's not really a drawback, its more like
    a security.
*/

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
