#include <QDebug>

#include "../Shared/networkcommands.h"
#include "../PokemonInfo/battlestructs.h"

#include "analyzer.h"
#include "serverconnection.h"

ServerConnection::ServerConnection(const GenericSocket &sock, int id) : id(id)
{
    relay = new Analyzer(sock, id);

    connect(relay, SIGNAL(disconnected()), SLOT(onError()));
    connect(relay, SIGNAL(connectionError(int,QString)), SLOT(onError()));
    connect(relay, SIGNAL(newBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)), SLOT(onNewBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)));

    connect(this, SIGNAL(destroyed()), relay, SLOT(deleteLater()));
}

void ServerConnection::onNewBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2)
{
    if (battles.contains(battleid)) {
        qWarning() << "Error, new battle with id " << battleid << " when already exists";
        return;
    }

    qDebug() << "new battle " << id << " starting!";

    emit newBattle(id, battleid, pb1, pb2, c, t1, t2);
}

void ServerConnection::notifyBattle(int id, int publicId, int opponent, const TeamBattle &team, const BattleConfiguration &config, const QString &tier)
{
    qDebug() << "Sending info about new battle " << publicId;

    relay->notify(EngageBattle, qint32(publicId), qint32(id), qint32(opponent), team, config, tier);
}

void ServerConnection::onError()
{
    emit error(id);
}
