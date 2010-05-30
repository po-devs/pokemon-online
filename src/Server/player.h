#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "analyze.h"
#include "../PokemonInfo/battlestructs.h"

class Challenge;
class BattleSituation;
/* a single player */
/***
  WARNING! Always use deleteLater!!
  This is due to Analyzer that requests a deleteLater too
***/



class Player : public QObject
{
    Q_OBJECT
    PROPERTY(int, rating);
    PROPERTY(bool, ladder);
    PROPERTY(bool, showteam);
    PROPERTY(QString, tier);
    PROPERTY(quint16, avatar);
    PROPERTY(QColor, color);
    PROPERTY(bool, battleSearch);
    PROPERTY(int, battleId);
    PROPERTY(QString, winningMessage);
    PROPERTY(QString, losingMessage);
    PROPERTY(QString, lastFindBattleIp);
public:
    BattleSituation *battle;
    QSet<int> battlesSpectated;

    enum State
    {
        NotLoggedIn=0,
        LoggedIn=1,
        Battling=2,
        Away = 4
    };

    Player(QTcpSocket *sock, int id);
    ~Player();

    /* returns all the regular info */
    TeamBattle &team();
    const TeamBattle &team() const;
    /* Converts the content of the TeamInfo to a basicInfo and returns it */
    BasicInfo basicInfo() const;

    /* Sends a message to the player */
    void sendMessage(const QString &mess);

    int id() const;
    QString name() const;
    QString ip() const;

    bool connected() const;
    bool isLoggedIn() const;
    bool battling() const;
    bool away() const;
    bool inSearchForBattle() const { return battleSearch(); }
    void cancelBattleSearch();
    void changeState(int newstate, bool on);
    int state() const;
    int auth() const;
    void setAuth (int newAuth);
    void setName (const QString & newName);

    int opponent () const;

    bool okForChallenge(int src) const;
    void addChallenge(Challenge *c, bool isChallenged);
    void removeChallenge(Challenge *c);
    void cancelChallenges();
    bool okForBattle() const;
    void spectateBattle(const QString &name0, const QString &name1, int battleId, bool doubles);
    void sendChallengeStuff(const ChallengeInfo &c);

    void doWhenDC();

    ChallengeInfo getChallengeInfo(int id); /* to get the battle info of a challenge received by that player */

    void startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf, bool doubles);
    void battleResult(int result, int winner, int loser);

    void kick();

    Analyzer& relay();
    const Analyzer& relay() const;

    PlayerInfo bundle() const;
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id, const QString &name);
    void sendChallenge(int source, int dest, const ChallengeInfo &desc);
    void battleFinished(int desc, int winner, int loser, bool rated, const QString& tier);
    void updated(int id);

    void battleMessage(int id,const BattleChoice &b);
    void battleChat(int id, const QString &);
    void info(int id, const QString &);
    void playerKick(int,int);
    void playerBan(int,int);
    void PMReceived(int, int, const QString &);
    void awayChange(int, bool);
    void spectatingRequested(int, int);
    void spectatingChat(int, int, const QString &chat);
    void spectatingStopped(int, int battleId);
    void findBattle(int,const FindBattleData&);
    void battleSearchCancelled(int);
public slots:
    void loggedIn(const TeamInfo &team,bool,bool, QColor);
    void recvMessage(const QString &mess);
    void recvTeam(const TeamInfo &team);
    void disconnected();
    void challengeStuff(const ChallengeInfo &c);
    void battleForfeited();
    void battleMessage(const BattleChoice &b);
    void battleChat(const QString &s);
    void registerRequest();
    void hashReceived(const QString &hash);
    void playerKick(int);
    void playerBan(int);
    void CPBan(const QString &name);
    void CPUnban(const QString &name);
    void CPTBan(const QString &name,const int &time);
    void receivePM(int, const QString&);
    void userInfoAsked(const QString& name);
    void giveBanList();
    void awayChange(bool away);
    void spectatingRequested(int id);
    void spectatingChat(int id, const QString &chat);
    void quitSpectating(int id);
    void ladderChange(bool);
    void showTeamChange(bool);
    void changeTier(const QString&);
    void findBattle(const FindBattleData&);
    void getRankingsByPage(const QString &tier, int page);
    void getRankingsByName(const QString &tier, const QString &name);
    void tUnban(QString name);
private:
    TeamBattle myteam;
    Analyzer myrelay;
    int myid;
    int myauth;
    QString myip;

    int m_opponent;
    int m_state;
    QString waiting_name; //For authentification procedures

    Challenge * challengedBy;

    QSet<Challenge*> challenged;

    enum AuthentificationState
    {
        Invalid,
        Partial,
        Success
    };

    AuthentificationState testAuthentification(const TeamInfo &team);
};

class TempBan : public QObject
{
    Q_OBJECT
public:
        TempBan(const QString& na,const int& ti);
        ~TempBan();
        void start();
        int time() const;
        QString name() const;
signals:
        void end(QString);

public slots:
        void done();

private:
        QTimer *mytimer;       
        QString myname;
        int mytime;
};

#endif // PLAYER_H
