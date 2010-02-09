#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "analyze.h"
#include "../PokemonInfo/battlestructs.h"

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
    enum State
    {
	NotLoggedIn,
	LoggedIn,
	Challenged,
	Battling
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
    void changeState(int newstate);
    int state() const;
    int auth() const;
    void setAuth (int newAuth);
    void setName (const QString & newName);

    int opponent () const;
    bool isChallenged() const;
    int challengedBy() const;
    bool hasChallenged(int id) const;

    void doWhenDC();
    /* Sends the challenge, returns false if can't even send the challenge */
    bool challenge(const ChallengeInfo &c);
    ChallengeInfo getChallengeInfo(int id); /* to get the battle info of a challenge received by that player */

    void startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf);
    void battleResult(int result, int winner, int loser);

    void sendChallengeStuff(int stuff, int otherparty);
    void addChallenge(const ChallengeInfo &c);
    void cancelChallenges();
    void resetChallenged();

    void kick();

    Analyzer& relay();
    const Analyzer& relay() const;
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id, const QString &name);

    void challengeStuff(const ChallengeInfo &, int idfrom, int idto);
    void battleFinished(int desc, int winner, int loser);

    void battleMessage(int id,const BattleChoice &b);
    void battleChat(int id, const QString &);
    void info(int id, const QString &);
    void playerKick(int,int);
    void playerBan(int,int);
    void PMReceived(int, int, const QString &);
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
private:
    TeamInfo myteam;
    Analyzer myrelay;
    int myid;
    int myauth;
    QString myip;

    QHash<int, ChallengeInfo> m_challenged;
    ChallengeInfo m_challengedby;
    int m_opponent;
    int m_state;
    QString waiting_name; //For authentification procedures

    void removeChallenge(int id);

    enum AuthentificationState
    {
        Invalid,
        Partial,
        Success
    };

    AuthentificationState testAuthentification(const TeamInfo &team);
};

#endif // PLAYER_H
