#ifndef BATTLEANALYZER_H
#define BATTLEANALYZER_H

#include "analyze.h"

class BattleAnalyzer : public Analyzer
{
    Q_OBJECT
public:
    BattleAnalyzer(QTcpSocket *sock);
signals:
    void sendBattleInfos(int bid, int p1, int p2, const TeamBattle &t, const BattleConfiguration &c, const QString &tier);
    void battleMessage(int bid, int p, const QByteArray &info);
    void battleResult(int bid, int result, int winner, int loser);
protected:
    void dealWithCommand(const QByteArray &command);
};

#endif // BATTLEANALYZER_H
