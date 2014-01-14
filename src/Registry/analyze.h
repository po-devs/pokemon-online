#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"
#include <Utilities/coreclasses.h>

/* Commands to dialog with the server */
namespace NetworkReg
{
#include "../Shared/networkcommands.h"
}

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(QTcpSocket *sock, int id);
    ~Analyzer();

    /* functions called by the reg */
    void sendRegistryAnnouncement(const QString &announcement);
    void sendServer(const QString &name, const QString &desc, quint16 numplayers, const QString &ip,quint16 max, quint16 port, bool passwordProtected);
    void sendServerListEnd(void);
    void sendInvalidName();
    void sendNameTaken();
    void sendAccept();

    bool isConnected() const;
    QString ip() const;

    /* Closes the connection */
    void close();

    /* Convenience functions to avoid writing a new one every time */
    inline void emitCommand(const QByteArray &command) {
        emit sendCommand(command);
    }

    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emitCommand(tosend);
    }

signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the reg */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(const QString &name, const QString &desc, quint16 num, quint16 max,quint16 nport, bool passwordProtected);
    void numChange(quint16 newnum);
    void nameChange(const QString &name);
    void descChange(const QString &desc);
    void maxChange(quint16);
    void passToggleChanged(bool);

    void disconnected();
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);
    void keepAlive();
private:
    Network &socket();
    const Network &socket() const;

    Network mysocket;
    QTimer *mytimer;
};

#endif // ANALYZE_H
