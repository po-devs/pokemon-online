#ifndef BATTLEANALYZER_H
#define BATTLEANALYZER_H

#include "../Utilities/baseanalyzer.h"

class TeamBattle;
class BattlePlayer;
class ChallengeInfo;
class BattleConfiguration;

class BattleAnalyzer : public BaseAnalyzer
{
    Q_OBJECT
public:
    BattleAnalyzer(QTcpSocket *sock);

    void startBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c,
                     const TeamBattle &t1, const TeamBattle &t2);
signals:
    void sendBattleInfos(int bid, int p1, int p2, const TeamBattle &t, const BattleConfiguration &c, const QString &tier);
    void battleMessage(int bid, int p, const QByteArray &info);
    void battleResult(int bid, int result, int winner, int loser);
public slots:
    void keepAlive();
protected:
    void dealWithCommand(const QByteArray &command);
};

#endif // BATTLEANALYZER_H
