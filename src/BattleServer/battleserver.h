#ifndef BATTLESERVER_H
#define BATTLESERVER_H

#include <QObject>
#include <QHash>

#include "../Utilities/contextswitch.h"
#include "../Utilities/asiosocket.h"

class BattleServerPluginManager;
class ServerConnection;
class BattlePlayer;
class ChallengeInfo;
class TeamBattle;

class BattleServer : public QObject
{
    Q_OBJECT
public:
    explicit BattleServer(QObject *parent = 0);
    
    void start();
    void changeDbMod(const QString &mod);
signals:
    
public slots:
    void print(const QString &s);
    void newConnection();

    void newBattle(int sid, int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2);
private:
    int freeid() const;

#ifndef BOOST_SOCKETS
    QTcpServer * server;
#else
    GenericSocket server;
    SocketManager manager;
#endif

    BattleServerPluginManager *pluginManager;
    ContextSwitcher battleThread;
    mutable int servercounter;

    QHash<int, ServerConnection*> connections;
};

#endif // BATTLESERVER_H
