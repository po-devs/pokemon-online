#ifndef DUALWIELDER_H
#define DUALWIELDER_H

#include <QObject>
#include "../QJson/qjson.h"
#include "../Utilities/coreclasses.h"
#include "../PokemonInfo/networkstructs.h"

class QWsSocket;
class Network;

class DualWielder : public QObject
{
    Q_OBJECT
public:
    DualWielder(QObject *parent = NULL);
    ~DualWielder();

    void init(QWsSocket *web, QString host, QHash<QString,QString> aliases);

    QString ip() const;
public slots:
    void readSocket(const QByteArray&);
    void readWebSocket(const QString&);
    void socketConnected();
    void socketDisconnected();
    void webSocketDisconnected();
signals:
    //Sends a command to the network
    void sendCommand(const QByteArray&);
private:
    QWsSocket *web;
    Network *network;
    QString mIp;
    QHash<QString,QString> aliases;

    QJson::Parser jparser;
    QJson::Serializer jserial;

    ProtocolVersion version;

    /* Convenience functions to avoid writing a new one every time */
    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emit sendCommand(tosend);
    }
};

#endif // DUALWIELDER_H
