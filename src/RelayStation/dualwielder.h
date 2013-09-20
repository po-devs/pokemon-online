#ifndef DUALWIELDER_H
#define DUALWIELDER_H

#include <QObject>
#include "../BattleManager/battleinput.h"
#include "battletojson.h"
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

    void init(QWsSocket *web, const QString &host, QHash<QString,QString> aliases, const QString& servers);

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
    QString servers;
    bool registryRead;
    /* IP aliases: when connecting to an IP in key, will connect to the value of the IP in values.
      Mainly used to do a public IP/localhost switch when the IP to connect to is the public IP of the own machine */
    QHash<QString,QString> aliases;

    /* Json parsers / serializers used */
    QJson::Parser jparser;
    QJson::Serializer jserial;

    /* Used to convert battle commands into JSON */
    BattleInput input;
    BattleToJson battleConverter;

    /* Ids to ignore (i.e. not relay) when info is received */
    QSet<int> toIgnore;
    /* Ids for which to get full information when they are relayed */
    QVector<int> importantPlayers;

    int myid;
    ProtocolVersion version;

    /* Convenience functions to avoid writing a new one every time */
    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly, version.version);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emit sendCommand(tosend);
    }
};

#endif // DUALWIELDER_H
