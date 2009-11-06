#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

class Network : public QObject
{
    Q_OBJECT
public:
    Network(QTcpSocket *sock);
    ~Network();
    /* Functions to reimplement:
	   isValid: returns whether the socket is valid or not!
	   close: closes the socket after writing the pending data
	   abort: abruptly closes the socket */
    int error() const;
    QString errorString() const;
public slots:
    void onReceipt();
    void send(const QByteArray &message);
signals:
    void isFull(QByteArray command);
    /* Signals to reimplement: error, disconnected */
private:
    /* internal socket */
    QTcpSocket *mysocket;
    /* internal variables for the protocol */
    bool commandStarted;
    quint16 remainingLength;

    QTcpSocket *socket();
    const QTcpSocket *socket() const;
};

#endif // NETWORK_H
