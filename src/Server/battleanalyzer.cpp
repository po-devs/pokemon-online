#include "../PokemonInfo/battlestructs.h"

#include "battleanalyzer.h"

using namespace NetworkServ;

BattleAnalyzer::BattleAnalyzer(GenericSocket sock) : Analyzer(sock, 0)
{
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
    default: break;
    }
}
