#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

class Network : public QObject
{
    Q_OBJECT
public:
    Network(QTcpSocket *sock, int id);
    ~Network();
    /* Functions to reimplement:
           isValid: returns whether the socket is valid or not!
           close: closes the socket after writing the pending data
           abort: abruptly closes the socket */
    int error() const;
    QString errorString() const;
    QString ip() const;
    bool isConnected() const;
    /* Trades bandwith for latency */
    void setLowDelay(bool lowDelay);

    void connectToHost(const QString & ip, quint16 port);

    void close();
    int id() const {return myid;}
public slots:
    void onReceipt();
    void onDisconnect();
    void manageError(QAbstractSocket::SocketError);
    void send(const QByteArray &message);
signals:
    void isFull(QByteArray command);
    void connected();
    void disconnected();
    void _error();
private:
    /* internal socket */
    QPointer<QTcpSocket> mysocket;
    /* internal variables for the protocol */
    bool commandStarted;
    quint16 remainingLength;
    /* errors stored when disconnected */
    int myerror;
    QString myerrorString;
    /* To keep track of things */
    int myid;
    /* Important.
       There were some crashes that drived me mad. If the server removes a socket, and theres still
       data pending, the socket would crash (but it took me time to discover that).
       So here it is! The thing to stop reading bullshit data. */
    bool stillValid;

    QString _ip;

    QTcpSocket *socket();
    const QTcpSocket *socket() const;
};

#endif // NETWORK_H
