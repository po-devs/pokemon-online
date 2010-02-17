#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "analyze.h"
#include "../PokemonInfo/battlestructs.h"

class Challenge;

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
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
    TeamInfo &team();
    const TeamInfo &team() const;
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
    void sendChallengeStuff(const ChallengeInfo &c);

    void doWhenDC();

    ChallengeInfo getChallengeInfo(int id); /* to get the battle info of a challenge received by that player */

    void startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf);
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
    void battleFinished(int desc, int winner, int loser);

    void battleMessage(int id,const BattleChoice &b);
    void battleChat(int id, const QString &);
    void info(int id, const QString &);
    void playerKick(int,int);
    void playerBan(int,int);
    void PMReceived(int, int, const QString &);
    void awayChange(int, bool);
public slots:
    void loggedIn(const TeamInfo &team);
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
    void receivePM(int, const QString&);
    void userInfoAsked(const QString& name);
    void giveBanList();
    void awayChange(bool away);
private:
    TeamInfo myteam;
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

/* Warning: some of the public methods delete the object (but they remove the references
   of the class from the players beforehand.

   That means you should only use 1 function at a time */
class Challenge : public QObject
{
    Q_OBJECT
public:
    Challenge (Player *source, Player *dest, const ChallengeInfo &c);

    void manageStuff(Player *source, const ChallengeInfo &c);
    void cancel(Player *p, bool refused = false);
    void onPlayerDisconnect(Player *p);

    int challenger() const { return src->id(); }
    int challenged() const { return dest->id(); }

    class Exception {

    };

signals:
    void battleStarted(int src, int dest, const ChallengeInfo &desc);
private:
    Player* src;
    Player* dest;
    ChallengeInfo desc;
};

#endif // PLAYER_H
