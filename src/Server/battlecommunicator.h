#ifndef BATTLECOMMUNICATOR_H
#define BATTLECOMMUNICATOR_H

#include <QObject>
#include <QString>
#include <QHash>

class Analyzer;
class BattleChoice;
class Player;
class ChallengeInfo;
class FullBattleConfiguration;

class BattleCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit BattleCommunicator(QObject *parent = 0);
    
    /* Number of ongoing battles */
    int count() const;
    bool contains(int battleid) const;

    /* Can we have battles? */
    bool valid() const;

    void startBattle(Player *p1, Player *p2, const ChallengeInfo &c, int id, int team1, int team2);

    /* Sent from the server */
    void playerForfeit(int battleid, int forfeiter);
    void removeBattle(int battleid);

    void addSpectator(int battleid, int id, const QString &name);
    void removeSpectator(int battleid, int id);

    FullBattleConfiguration *battle(int battleid);
signals:
    void info(const QString &message);
    void error();
    void battleInfo(int,int,QByteArray);
    void battleFinished(int,int,int,int);
public slots:
    void connectToBattleServer();
    void battleConnected();
    void battleConnectionError();

    void battleMessage(int player, int battle, const BattleChoice &choice);
    void resendBattleInfos(int player, int battle);
    void battleChat(int player, int battle, const QString &chat);
    void spectatingChat(int player, int battle, const QString &chat);
private:
    Analyzer* battleserver_connection;

    QHash<int, FullBattleConfiguration*> mybattles;
};

#endif // BATTLECOMMUNICATOR_H
