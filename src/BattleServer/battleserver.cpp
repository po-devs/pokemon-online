#include <stdexcept>
#include <QtNetwork>

#include <Utilities/asiosocket.h>
#include <PokemonInfo/pokemoninfo.h>
#include <PokemonInfo/movesetchecker.h>

#include "abilities.h"
#include "items.h"
#include "moves.h"
#include "rbymoves.h"
#include "battlebase.h"
#include "serverconnection.h"
#include "battleserver.h"
#include "pluginmanager.h"

BattleServer::BattleServer(QObject *parent) :
    QObject(parent), servercounter(0), closeOnDc(false)
{
}

void BattleServer::start(int port, bool closeOnDc)
{
    print("Starting Battle Server...");

    print("Initialising Pokemon & Battle database");

    PokemonInfoConfig::setFillMode(FillMode::Server);

    changeDbMod("");

    MoveEffect::init();
    RBYMoveEffect::init();
    ItemEffect::init();
    AbilityEffect::init();

    print("...Done!");

    pluginManager = new BattleServerPluginManager();

    closeOnDc = closeOnDc;

#ifndef BOOST_SOCKETS
    server = new QTcpServer();
    bool listenSuccess = server->listen(QHostAddress::LocalHost, port);
#else
    server = manager.createServerSocket();
    bool listenSuccess = server->listen(port, "127.0.0.1");
#endif

    if (listenSuccess)
    {
        print(QString("Starting to listen to port %1").arg(port));
    } else {
        print(QString("Unable to listen to port %1").arg(port));
        ::exit(1);
    }

#ifndef BOOST_SOCKETS
    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
#else
    connect(&*server, SIGNAL(active()), SLOT(newConnection()));
    manager.start();
#endif

    battleThread.start();
    print("Battle thread started");
}

void BattleServer::changeDbMod(const QString &mod)
{
    battleThread.pause();

    PokemonInfoConfig::changeMod(mod);

    /* Really useful for headless servers */
    GenInfo::init("db/gens/");
    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
    GenderInfo::init("db/genders/"); //needed by battlelogs plugin

    PokemonInfo::loadStadiumTradebacks();

    battleThread.unpause();
}


void BattleServer::print(const QString &s)
{
    qDebug() << s;
}

void BattleServer::newConnection()
{
    GenericSocket newconnection = server->nextPendingConnection();

    if (!newconnection)
        return;

    int id = freeid();
#ifndef BOOST_SOCKETS
    QString ip = newconnection->peerAddress().toString();
#else
    QString ip = newconnection->ip();
#endif

    print(QString("Received new connection on slot %1 from %2").arg(id).arg(ip));
    ServerConnection *conn = connections[id] = new ServerConnection(newconnection, id);

    connect(conn, SIGNAL(newBattle(int,int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)), SLOT(newBattle(int,int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)));
    connect(conn, SIGNAL(error(int)), SLOT(onError(int)));
    connect(conn, SIGNAL(modChanged(QString)), SLOT(modChanged(QString)));
    connect(conn, SIGNAL(loadPlugin(QString)), SLOT(loadPlugin(QString)));
    connect(conn, SIGNAL(unloadPlugin(QString)), SLOT(unloadPlugin(QString)));
}

void BattleServer::onError(int id)
{
    if (!connections.contains(id)) {
        return;
    }

    print(QString("Disonnection from slot %1").arg(id));
    connections.take(id)->deleteLater();

    if (closeOnDc) {
        ::exit(0);
    }
}

void BattleServer::modChanged(const QString &mod)
{
    if (PokemonInfoConfig::currentMod() == mod) {
        return;
    }

    print(QString("Database mod changed: %1").arg(mod));
    changeDbMod(mod);
}

void BattleServer::newBattle(int sid, int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2)
{
    BattleBase *battle;

    if (c.gen <= 1) {
        battle = (BattleBase*)new BattleRBY(pb1, pb2, c, battleid, t1, t2, pluginManager);
    } else {
        battle = new BattleSituation(pb1, pb2, c, battleid, t1, t2, pluginManager);
    }

    ServerConnection *conn = connections[sid];

    conn->battles.insert(battleid, battle);
    connect(battle, SIGNAL(sendBattleInfos(int,int,int,TeamBattle,BattleConfiguration,QString)), conn, SLOT(notifyBattle(int,int,int,TeamBattle,BattleConfiguration,QString)));
    connect(battle, SIGNAL(battleInfo(int,int,QByteArray)), conn, SLOT(notifyInfo(int,int,QByteArray)));
    connect(battle, SIGNAL(battleFinished(int,int,int,int)), conn, SLOT(notifyFinished(int,int,int,int)));
    connect(conn, SIGNAL(destroyed()), battle, SLOT(deleteLater()));

    battle->start(battleThread);
}

int BattleServer::freeid() const
{
    do {
        ++servercounter;
    } while (connections.contains(servercounter) || servercounter == 0 || servercounter == -1); /* 0, -1 are reserved */

    return servercounter;
}

void BattleServer::loadPlugin(const QString &path)
{
    try {
        pluginManager->addPlugin(path);
    } catch (const std::runtime_error &e) {
        qDebug() << e.what();
    }
}

void BattleServer::unloadPlugin(const QString &name)
{
    pluginManager->freePlugin(name);
}
