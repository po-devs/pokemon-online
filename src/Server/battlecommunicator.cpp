#include <QTimer>

#include "../Shared/networkcommands.h"
#include "../Shared/battlecommands.h"

#include <Utilities/exesuffix.h>
#include <PokemonInfo/battlestructs.h>

#include "tiermachine.h"
#include "tier.h"
#include "battleanalyzer.h"
#include "player.h"
#include "battlecommunicator.h"

static const QString processErrorMessages[] = {
    "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.",
    "The process crashed some time after starting successfully.",
    "The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again.",
    "An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.",
    "An error occurred when attempting to read from the process. For example, the process may not be running.",
    "An unknown error occurred. This is the default return value of error()."
};

BattleCommunicator::BattleCommunicator(QObject *parent) :
    QObject(parent), relay(nullptr)
{
    QTimer::singleShot(3000, this, SLOT(connectToBattleServer()));
    battleServer = new QProcess(this);

    wasConnected = false;
    silent = false;
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
    mybattles[id]->name[0] = p1->name();
    mybattles[id]->name[1] = p2->name();

    if (TierMachine::obj()->exists(tier)) {
        Tier & t = TierMachine::obj()->tier(tier);

        BattlePlayer pb1(p1->name(), p1->id(), p1->rating(p1->team(team1).tier), p1->avatar(), p1->winningMessage(), p1->losingMessage(),
                         p1->tieMessage(), t.getMaxLevel(), t.restricted(p1->team(team1)), t.maxRestrictedPokes, t.numberOfPokemons);
        BattlePlayer pb2(p2->name(), p2->id(), p2->rating(p2->team(team2).tier), p2->avatar(), p2->winningMessage(), p2->losingMessage(),
                         p2->tieMessage(), t.getMaxLevel(), t.restricted(p2->team(team2)), t.maxRestrictedPokes, t.numberOfPokemons);

        relay->startBattle(id, pb1, pb2, c, p1->team(team1), p2->team(team2));
    } else {
        BattlePlayer pb1(p1->name(), p1->id(), p1->rating(p1->team(team1).tier), p1->avatar(), p1->winningMessage(), p1->losingMessage(),
                         p1->tieMessage());
        BattlePlayer pb2(p2->name(), p2->id(), p2->rating(p2->team(team2).tier), p2->avatar(), p2->winningMessage(), p2->losingMessage(),
                         p2->tieMessage());

        relay->startBattle(id, pb1, pb2, c, p1->team(team1), p2->team(team2));
    }

    p1->addBattle(id);
    p2->addBattle(id);
}

void BattleCommunicator::loadPlugin(const QString &path)
{
    if (relay) {
        relay->notify(LoadPlugin, true, path);
    }
}

void BattleCommunicator::unloadPlugin(const QString &name)
{
    if (relay) {
        relay->notify(LoadPlugin, false, name);
    }
}

void BattleCommunicator::playerForfeit(int battleid, int forfeiter)
{
    relay->notify(BattleFinished, qint32(battleid), uchar(Forfeit), qint32(forfeiter));

    /* Manually send the forfeit to everyone since we're going to remove the battle soon and so not forward anymore of its messages */
    if (contains(battleid)) {
        mybattles[battleid]->finished() = true;
        showResult(battleid, Forfeit, forfeiter);
    }
}

void BattleCommunicator::showResult(int battleid, int result, int loser)
{
    QByteArray command;
    DataStream stream(&command, QIODevice::WriteOnly);

    stream << uchar(BattleCommands::BattleEnd) << qint8(mybattles[battleid]->opponent(mybattles[battleid]->spot(loser))) << uchar(result);

    emit battleInfo(battleid, mybattles[battleid]->id(0), command);
    emit battleInfo(battleid, mybattles[battleid]->id(1), command);

    foreach(int spectator, mybattles[battleid]->spectators) {
        emit battleInfo(battleid, spectator, command);
    }
}

void BattleCommunicator::removeBattle(int battleid)
{
    delete mybattles.take(battleid);

    relay->notify(BattleFinished, qint32(battleid), uchar(Close));
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

void BattleCommunicator::killServer()
{
    if (battleServer->state() == QProcess::Running) {
        battleServer->kill();
    }
}

bool BattleCommunicator::startServer()
{
    if (battleServer->state() == QProcess::Starting || battleServer->state() == QProcess::Running) {
        return false;
    }

    emit info("Starting battle server.");
    battleServer->start("./BattleServer" SUFFIX " -p 5096 -c");
    connect(battleServer, SIGNAL(started()), this, SLOT(battleServerStarted()));
    connect(battleServer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(battleServerError(QProcess::ProcessError)));
    return true;
}

void BattleCommunicator::battleServerStarted()
{
    emit info("Battle server started.");
}

void BattleCommunicator::battleServerError(QProcess::ProcessError error)
{
    emit info(QString("Battle server error: %1").arg(processErrorMessages[error]));
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

    if (!silent) {
        emit info("Connecting to battle server on port 5096...");
    } else {
        silent = false;
    }

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
    wasConnected = true;
    changeMod(mod);
}

void BattleCommunicator::battleConnectionError()
{
    removeBattles();
    // Only send messages if there was previously a connection
    // or a server is already running (which means it should've connected).
    if (wasConnected) {
        emit info("Error when connecting to the battle server. Will try again in 10 seconds");

        wasConnected = false;
        emit battleConnectionLost();
    } else {
        silent = true;
    }

    emit error();
    QTimer::singleShot(10000, this, SLOT(connectToBattleServer()));
}


void BattleCommunicator::battleMessage(int player, int battle, const BattleChoice &choice)
{
    if (!contains(battle)) {
        return;
    }

    relay->notifyChoice(battle, player, choice);
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
    if (!contains(b)) {
        return;
    }

    emit sendBattleInfos(b, p1, p2, t, c, s);
}

void BattleCommunicator::filterBattleInfo(int battleid, int player, const QByteArray &info)
{
    if (!contains(battleid)) {
        return;
    }

    FullBattleConfiguration *battle = mybattles[battleid];

    /* Show variation here */
    if (battle->rated() && info.length() > 0 && (battle->id(0) == player || battle->id(1) == player) && info[0] == BattleCommands::Rated) {
        QPair<int,int> firstChange = TierMachine::obj()->pointChangeEstimate(battle->name[battle->spot(player)], battle->name[battle->opponent(battle->spot(player))], battle->tier());

        emit battleInfo(battleid, player, pack(BattleCommands::PointEstimate, battle->spot(player), qint8(firstChange.first), qint8(firstChange.second)));
    }

    emit battleInfo(battleid, player, info);
}

void BattleCommunicator::filterBattleResult(int b, int r, int w, int l)
{
    if (!contains(b)) {
        return;
    }

    battle(b)->finished() = true;

    emit battleFinished(b,r,w,l);
}

void BattleCommunicator::changeMod(const QString &mod)
{
    this->mod = mod;

    if (relay) {
        relay->notify(DatabaseMod, mod);
    }
}

void BattleCommunicator::removeBattles()
{
    /* Removing all battles */
    foreach(int battle, mybattles.keys()) {
        mybattles[battle]->finished() = true;
        showResult(battle, Tie, mybattles[battle]->id(0));

        emit battleFinished(battle, Tie, mybattles[battle]->id(0), mybattles[battle]->id(1));
        emit battleFinished(battle, Close, mybattles[battle]->id(0), mybattles[battle]->id(1));
    }
}
