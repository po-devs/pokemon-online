#include "../Shared/networkcommands.h"
#include "../PokemonInfo/battlestructs.h"

#include "battleanalyzer.h"

BattleAnalyzer::BattleAnalyzer(QTcpSocket *sock) : BaseAnalyzer(sock, 0)
{
}

void BattleAnalyzer::startBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2)
{
    notify(EngageBattle, qint32(battleid), pb1, pb2, c, t1.fullSerial(), t2.fullSerial());
}

void BattleAnalyzer::notifyChoice(int battle, int player, const BattleChoice &choice)
{
    notify (BattleMessage, qint32(battle), qint32(player), choice);
}

void BattleAnalyzer::dealWithCommand(const QByteArray &commandline)
{
    DataStream in (commandline);
    uchar command;

    in >> command;

    switch (command) {
    case EngageBattle: {
        qint32 bid;
        qint32 p1, p2;
        TeamBattle t;
        BattleConfiguration c;
        QString tier;

        in >> bid >> p1 >> p2 >> t >> c >> tier;

        emit sendBattleInfos(bid, p1, p2, t, c, tier);
        break;
    }
    case BattleMessage: {
        qint32 bid, p;
        QByteArray message;

        in >> bid >> p >> message;

        emit battleMessage(bid, p, message);
        break;
    }
    case BattleFinished: {
        qint32 bid, result, winner, loser;

        in >> bid >> result >> winner >> loser;

        emit battleResult(bid, result, winner, loser);
        break;
    }
    default: break;
    }
}

void BattleAnalyzer::keepAlive()
{
    notify(KeepAlive);
}
