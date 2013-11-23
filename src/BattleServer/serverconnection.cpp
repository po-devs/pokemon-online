#include <QDebug>

#include "analyzer.h"
#include "serverconnection.h"

ServerConnection::ServerConnection(const GenericSocket &sock, int id) : id(id)
{
    myrelay = new Analyzer(sock, id);

    connect(myrelay, SIGNAL(newBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)), SLOT(onNewBattle(int,BattlePlayer,BattlePlayer,ChallengeInfo,TeamBattle,TeamBattle)));
}

void ServerConnection::onNewBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2)
{
    if (battles.contains(battleid)) {
        qWarning() << "Error, new battle with id " << battleid << " when already exists";
        return;
    }

    emit newBattle(id, battleid, pb1, pb2, c, t1, t2);
}
