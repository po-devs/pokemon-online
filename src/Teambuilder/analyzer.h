#ifndef ANALYZER_H
#define ANALYZER_H

#include <QtCore>

class Network;
struct pendingRequests;

/* Analyzer: Piles up request from both side (main class & Network class) and executes them when push() and
    pull() are called. This avoids multithreading conflicts : only this one class has to manage multihreading.

    This class also acts as a protocol encoder / decoder.

    The messages are a 8 bit command followed by additional data */

class Analyzer : public QObject
{
    Q_OBJECT
public:
    /* Connects all the necessary slots/signals between this class & Analyzer */
    Analyzer(Network &network);
public slots:
    /* Sends all the requests from the Main Class to the Network class*/
    void push();
    /* Sends all the feedback from the Network class to the Main Class*/
    void pull();

    /* close Connection: sent by the main class to close a connection */
    void closeConnection(int id);
    /* Sends a command with data */
    void sendCommand(int id, quint8 command, const QByteArray &data);
    /* Sent by the network class to say a connection is broken */
    void brokenConnection(int id);
    /* Data was received */
    void dataReceived(int id, const QByteArray &data);
signals:
    /* Signals to tell the main class what happened */
    void commandReceived(int socketId, quint8 command, QByteArray data);
    void broken(int id);
    /* Signals to tell the network class what to do */
    void close(int id);
    void connectTo(int id, QString ip, quint16 port);
    void sendData(QByteArray data);
private:
    QMap<int, pendingRequests> requestsFromClient;
    QMutex clientMut;

    QMap<int, pendingRequests> requestsFromNetork;
    QMutex networkMut;
};

struct pendingRequests
{
    QDataStream rawInformation;
    /* Was a request to close the connection sent / Was the connection closed? */
    bool close;

    /* initializes close to false */
    pendingRequests();

    /* piles up a request */
    void pileupRequest(quint8 command, const QByteArray &data);
    void requestClose();
    /* Returns true if there is a request to pop */
    bool popbackRequest(quint8 &command, QByteArray &data);
    bool closeRequested() const;
};

#endif // ANALYZER_H
