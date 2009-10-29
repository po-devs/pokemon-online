#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>

/* Manages sockets and store them with ids.

    If any errors occurs it sends a signal broken(int id) where id is the id
    of the corresponding socket and then removes the socket

    This class is really low-level, it doesn't analyze what goes throught sockets,
    it just reads & sends */

class TcpSocket;

class Network : public QObject
{
    Q_OBJECT
public:
    Network();
public slots:
    /* Creates a socket connecting to ip, and with the given id. If any other socket
        shares the same id it's forcefully removed */
    void connectTo(int id, const QString &ip, quint16 port);
    /* Send data to the socket with that ID, emits broken errors */
    void sendData(int socketId, const QByteArray &data);
    /* Closes a socket */
    void close(int socketId);
signals:
    /* Signal emitted when an error occured, it means also Network stopped the connection */
    void broken(int id);
    /* Signal emitted when data was received */
    void receiveData(int id, QByteArray data);
private:
    /* Returns the socket corresponding to the id */
    QTcpSocket & socket(int id);
    
    /* where are stored the sockets */
    QMap<int, TcpSocket> mySockets;
};

/* Own implementation of TcpSocket, added the field "id" */
class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    /* Sets the id of the socket to -1 by default */
    TcpSocket();

    /* Changes / gets the id of the socket */
    void setId (int newid);
    int id() const;
};

#endif // NETWORK_H
