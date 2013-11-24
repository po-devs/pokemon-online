#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QHash>

#include "../Utilities/asiosocket.h"

class Analyzer;
class BattleBase;
class BattlePlayer;
class ChallengeInfo;
class TeamBattle;
class BattleConfiguration;

class ServerConnection : public QObject
{
    friend class BattleServer;
    Q_OBJECT
public:
    ServerConnection(const GenericSocket &sock, int id);
    
//    Analyzer& relay();
//    const Analyzer& relay() const;

signals:
    void newBattle(int sid, int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2);
    void error(int id);
public slots:
    void onNewBattle(int battleid, const BattlePlayer &pb1, const BattlePlayer &pb2, const ChallengeInfo &c, const TeamBattle &t1, const TeamBattle &t2);
    void notifyBattle(int id, int publicId, int opponent, const TeamBattle &team, const BattleConfiguration &config, const QString &tier);
    void onError();
private:
    Analyzer *relay;
    QHash<int, BattleBase *> battles;

    int id;
};

#endif // SERVERCONNECTION_H
