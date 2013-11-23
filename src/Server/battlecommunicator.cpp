#include <QTimer>

#include "analyze.h"
#include "player.h"
#include "battlecommunicator.h"

BattleCommunicator::BattleCommunicator(QObject *parent) :
    QObject(parent), battleserver_connection(nullptr)
{
    system("./BattleServer");

    QTimer::singleShot(5000, this, SLOT(connectToBattleServer()));
}

int BattleCommunicator::count() const
{
    return mybattles.count();
}

bool BattleCommunicator::contains(int battleid) const
{
    return mybattles.contains(battleid);
}

bool BattleCommunicator::valid() const
{
    return battleserver_connection && battleserver_connection->isConnected();
}

void BattleCommunicator::startBattle(Player *p1, Player *p2, const ChallengeInfo &c, int id, int team1, int team2)
{
    QString tier = p1->team(team1).tier == p2->team(team2).tier ? p1->team(team1).tier : QString("Mixed %1").arg(GenInfo::Version(p1->team(team1).gen));

    mybattles.insert(id, new FullBattleConfiguration(id, p1->id(), p2->id(), tier, c));

    /*
    BattleBase *battle;
    if (c.gen <= 1) {
        battle = (BattleBase*)new BattleRBY(*player(id1), *player(id2), c, id, team1, team2, pluginManager);
    } else {
        battle = new BattleSituation(*player(id1), *player(id2), c, id, team1, team2, pluginManager);
    }

    p1->startBattle(id, id2, battle->pubteam(id1), battle->configuration(), battle->tier());
    p2->startBattle(id, id1, battle->pubteam(id2), battle->configuration(), battle->tier());*/
}

void BattleCommunicator::playerForfeit(int battleid, int forfeiter)
{
    /* Todo: forward forfeit info to the battles engine */
}

void BattleCommunicator::removeBattle(int battleid)
{
    delete mybattles.take(battleid);

    /* Todo: maybe notify battle server! */
}

void BattleCommunicator::addSpectator(int idOfBattle, int id, const QString &name)
{
    mybattles[idOfBattle]->spectators.insert(id);

    /* Todo: forward to battle server */
}

void BattleCommunicator::removeSpectator(int idOfBattle, int id)
{
    if (!contains(idOfBattle)) {
        qFatal("Critical bug needing to be solved: BattleCommunicator::removeSpectator, player %d and non-existent battle %d", id, idOfBattle);
    } else {
        mybattles[idOfBattle]->spectators.remove(id);

        /* Todo: Forward to battle server */
    }
}

FullBattleConfiguration *BattleCommunicator::battle(int battleid)
{
    if (!contains(battleid)) {
        qFatal("Fatal! Looking for non existent battle %d", battleid);
    }
    return mybattles.value(battleid);
}

void BattleCommunicator::battleMessage(int player, int battle, const BattleChoice &choice)
{
    if (!contains(battle)) {
        return;
    }

//    mybattles[battle]->battleChoiceReceived(player, choice);
}

void BattleCommunicator::resendBattleInfos(int player, int battle)
{
    if (!contains(battle)) {
        return;
    }

    //mybattles[battle]->addSpectator(this->player(player));
    //mybattles[battle]->spectators.insert(player);
}

void BattleCommunicator::battleChat(int player, int battle, const QString &chat)
{
    if (!mybattles.contains(battle)) {
        return;
    }

    //mybattles[battle]->battleChat(player, chat);
}

void BattleCommunicator::spectatingChat(int player, int battle, const QString &chat)
{
    if (!mybattles.contains(battle)) {
        return;
    }
    //mybattles[battle]->spectatingChat(player, chat);
}

void BattleCommunicator::connectToBattleServer()
{
    if (battleserver_connection) {
        if (battleserver_connection->isConnected()) {
            return;
        }
        else
            battleserver_connection->deleteLater();
    }

    battleserver_connection = nullptr;

    emit info("Connecting to battle server on port 5096...");

    QTcpSocket * s = new QTcpSocket(nullptr);
    s->connectToHost("localhost", 5096);

    connect(s, SIGNAL(connected()), this, SLOT(battleConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(battleConnectionError()));

    battleserver_connection = new Analyzer(s,0);
}

void BattleCommunicator::battleConnected()
{
    emit info("Connected to battle server!");
}

void BattleCommunicator::battleConnectionError()
{
    emit info("Error when connecting to the battle server. Will try again in 10 seconds");
    emit error();

    QTimer::singleShot(10000, this, SLOT(connectToBattleServer()));
}
