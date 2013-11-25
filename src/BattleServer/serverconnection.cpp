#include <QDebug>

#include "../Shared/networkcommands.h"
#include "../PokemonInfo/battlestructs.h"

#include "battlebase.h"
#include "analyzer.h"
#include "serverconnection.h"

ServerConnection::ServerConnection(const GenericSocket &sock, int id) : id(id)
{
    relay = new Analyzer(sock, id);

    connect(relay, SIGNAL(disconnected()), SLOT(onError()));
    connect(relay, SIGNAL(connectionError(int,QString)), SLOT(onError()));
    connect(relay, SIGNAL(newBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)), SLOT(onNewBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)));
    connect(relay, SIGNAL(battleChat(int,int,QString)), SLOT(message(int,int,QString)));
    connect(relay, SIGNAL(spectatingChat(int,int,QString)), SLOT(spectatorMessage(int,int,QString)));
    connect(relay, SIGNAL(playerChoice(int,int,BattleChoice)), SLOT(choice(int,int,BattleChoice)));
    connect(relay, SIGNAL(spectating(int,bool,int,QString)), SLOT(spectate(int,bool,int,QString)));

    connect(this, SIGNAL(destroyed()), relay, SLOT(deleteLater()));
}

void ServerConnection::onNewBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2)
{
    if (battles.contains(battleid)) {
        qWarning() << "Error, new battle with id " << battleid << " when already exists";
        return;
    }

    emit newBattle(id, battleid, pb1, pb2, c, t1, t2);
}

void ServerConnection::spectate(int battleid, bool spectate, int player, const QString &name)
{
    if (!battles.contains(battleid)) {
        qWarning() << "Error, spectating in non existing batte " << battleid;
        return;
    }

    if (spectate) {
        battles[battleid]->addSpectator(QPair<int,QString>(player, name));
    } else {
        battles[battleid]->removeSpectator(player);
    }
}

void ServerConnection::choice(int battleid, int player, const BattleChoice &choice)
{
    if (!battles.contains(battleid)) {
        qWarning() << "Error, choice in non existing batte " << battleid;
        return;
    }

    battles[battleid]->battleChoiceReceived(player, choice);
}

void ServerConnection::message(int battleid, int player, const QString &chat)
{
    if (!battles.contains(battleid)) {
        qWarning() << "Error, chat in non existing batte " << battleid;
        return;
    }

    battles[battleid]->battleChat(player, chat);
}

void ServerConnection::spectatorMessage(int battleid, int player, const QString &chat)
{
    if (!battles.contains(battleid)) {
        qWarning() << "Error, spectator chat in non existing batte " << battleid;
        return;
    }

    battles[battleid]->spectatingChat(player, chat);
}

void ServerConnection::notifyBattle(int id, int publicId, int opponent, const TeamBattle &team, const BattleConfiguration &config, const QString &tier)
{
    relay->notify(EngageBattle, qint32(publicId), qint32(id), qint32(opponent), team, config, tier);
}

void ServerConnection::notifyInfo(int battle, int player, const QByteArray &info)
{
    relay->notify(BattleMessage, qint32(battle), qint32(player), info);
}

void ServerConnection::notifyFinished(int battle, int result, int winner, int loser)
{
    relay->notify(BattleFinished, qint32(battle), qint32(result), qint32(winner), qint32(loser));
}

void ServerConnection::onError()
{
    emit error(id);
}
