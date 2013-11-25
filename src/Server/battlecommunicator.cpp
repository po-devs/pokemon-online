#include <QTimer>

#include "../Shared/networkcommands.h"
#include "../PokemonInfo/battlestructs.h"

#include "tiermachine.h"
#include "tier.h"
#include "battleanalyzer.h"
#include "player.h"
#include "battlecommunicator.h"

BattleCommunicator::BattleCommunicator(QObject *parent) :
    QObject(parent), relay(nullptr)
{
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
    return relay && relay->isConnected();
}

void BattleCommunicator::startBattle(Player *p1, Player *p2, const ChallengeInfo &c, int id, int team1, int team2)
{
    if (!valid()) {
        qFatal("Starting a battle when no valid connections");
    }

    QString tier = p1->team(team1).tier == p2->team(team2).tier ? p1->team(team1).tier : QString("Mixed %1").arg(GenInfo::Version(p1->team(team1).gen));

    mybattles.insert(id, new FullBattleConfiguration(id, p1->id(), p2->id(), tier, c));

    if (TierMachine::obj()->exists(tier)) {
        Tier & t = TierMachine::obj()->tier(tier);

        BattlePlayer pb1 = {
            p1->name(), p1->winningMessage(), p1->losingMessage(), p1->tieMessage(), p1->rating(p1->team(team1).tier), p1->avatar(), p1->id(),
            (quint8)t.getMaxLevel(), (quint8)t.restricted(p1->team(team1)), (quint8)t.maxRestrictedPokes, (quint8)t.numberOfPokemons
        };
        BattlePlayer pb2 = {
            p2->name(), p2->winningMessage(), p2->losingMessage(), p2->tieMessage(), p2->rating(p1->team(team1).tier), p2->avatar(), p2->id(),
            (quint8)t.getMaxLevel(), (quint8)t.restricted(p1->team(team1)), (quint8)t.maxRestrictedPokes, (quint8)t.numberOfPokemons
        };

        relay->notify(EngageBattle, id, pb1, pb2, c, p1->team(team1), p2->team(team2));
    } else {
        BattlePlayer pb1 = {
            p1->name(), p1->winningMessage(), p1->losingMessage(), p1->tieMessage(), p1->rating(p1->team(team1).tier), p1->avatar(), p1->id(),
            (quint8)100, (quint8)0, (quint8)0, (quint8)6
        };
        BattlePlayer pb2 = {
            p2->name(), p2->winningMessage(), p2->losingMessage(), p2->tieMessage(), p2->rating(p1->team(team1).tier), p2->avatar(), p2->id(),
            (quint8)100, (quint8)0, (quint8)0, (quint8)6
        };

        relay->notify(EngageBattle, id, pb1, pb2, c, p1->team(team1), p2->team(team2));
    }

    p1->addBattle(id);
    p2->addBattle(id);

    /*


//    if (rated()) {
//        QPair<int,int> firstChange = TierMachine::obj()->pointChangeEstimate(team(0).name, team(1).name, tier());
//        QPair<int,int> secondChange = TierMachine::obj()->pointChangeEstimate(team(1).name, team(0).name, tier());

//        notify(Player1, PointEstimate, Player1, qint8(firstChange.first), qint8(firstChange.second));
//        notify(Player2, PointEstimate, Player2, qint8(secondChange.first), qint8(secondChange.second));
//    }

*/
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

void BattleCommunicator::addSpectator(int battle, int id, const QString &name)
{
    mybattles[battle]->spectators.insert(id);

    relay->notify(SpectateBattle, qint32(battle), true, qint32(id), name);
}

void BattleCommunicator::removeSpectator(int idOfBattle, int id)
{
    if (!contains(idOfBattle)) {
        qFatal("Critical bug needing to be solved: BattleCommunicator::removeSpectator, player %d and non-existent battle %d", id, idOfBattle);
    } else {
        mybattles[idOfBattle]->spectators.remove(id);

        relay->notify(SpectateBattle, qint32(idOfBattle), false, qint32(id));
    }
}

FullBattleConfiguration *BattleCommunicator::battle(int battleid)
{
    if (!contains(battleid)) {
        qFatal("Fatal! Looking for non existent battle %d", battleid);
    }
    return mybattles.value(battleid);
}

void BattleCommunicator::connectToBattleServer()
{
    if (relay) {
        if (relay->isConnected()) {
            return;
        }
        else
            relay->deleteLater();
    }

    relay = nullptr;

    emit info("Connecting to battle server on port 5096...");

    QTcpSocket * s = new QTcpSocket(nullptr);
    s->connectToHost("localhost", 5096);

    connect(s, SIGNAL(connected()), this, SLOT(battleConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(battleConnectionError()));

    relay = new BattleAnalyzer(s);

    connect(relay, SIGNAL(sendBattleInfos(int,int,int,TeamBattle,BattleConfiguration,QString)), SLOT(filterBattleInfos(int,int,int,TeamBattle,BattleConfiguration,QString)));
    connect(relay, SIGNAL(battleMessage(int,int,QByteArray)), SLOT(filterBattleInfo(int,int,QByteArray)));
    connect(relay, SIGNAL(battleResult(int,int,int,int)), SLOT(filterBattleResult(int,int,int,int)));
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


void BattleCommunicator::battleMessage(int player, int battle, const BattleChoice &choice)
{
    if (!contains(battle)) {
        return;
    }

    relay->notify(BattleMessage, qint32(battle), qint32(player), choice);
}

void BattleCommunicator::resendBattleInfos(int player, int battle)
{
    if (!contains(battle)) {
        return;
    }

    relay->notify(SpectateBattle, qint32(battle), true, qint32(player));
}

void BattleCommunicator::battleChat(int player, int battle, const QString &chat)
{
    if (!contains(battle)) {
        return;
    }

    relay->notify(BattleChat, qint32(battle), qint32(player), chat);
}

void BattleCommunicator::spectatingChat(int player, int battle, const QString &chat)
{
    if (!contains(battle)) {
        return;
    }

    relay->notify(SpectatingBattleChat, qint32(battle), qint32(player), chat);
}

void BattleCommunicator::filterBattleInfos(int b, int p1, int p2, const TeamBattle &t, const BattleConfiguration &c, const QString &s)
{
    emit info(QString("infos about battle %1 received").arg(b));
    if (!contains(b)) {
        return;
    }

    emit info(QString("infos about battle %1 received!").arg(b));
    emit sendBattleInfos(b, p1, p2, t, c, s);
}

void BattleCommunicator::filterBattleInfo(int battle, int player, const QByteArray &info)
{
    if (!contains(battle)) {
        return;
    }

    emit battleInfo(battle, player, info);
}

void BattleCommunicator::filterBattleResult(int b, int r, int w, int l)
{
    if (!contains(b)) {
        return;
    }

    emit battleFinished(b,r,w,l);
}