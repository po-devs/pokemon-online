#ifndef BATTLEANALYZER_H
#define BATTLEANALYZER_H

#include "analyze.h"

class BattleAnalyzer : public Analyzer
{
    Q_OBJECT
public:
    BattleAnalyzer(GenericSocket sock);
signals:
    void sendBattleInfos(int bid, int p1, int p2, const TeamBattle &t, const BattleConfiguration &c, const QString &tier);
protected:
    void dealWithCommand(const QByteArray &command);
};

#endif // BATTLEANALYZER_H
