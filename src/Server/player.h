#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"
#include "playerinterface.h"
#include "sfmlsocket.h"

class Challenge;
class BattleSituation;
class Analyzer;

/* a single player */
/***
  WARNING! Always use deleteLater!!
  This is due to Analyzer that requests a deleteLater too
***/

class Player : public QObject, public PlayerInterface
{
    Q_OBJECT

    PROPERTY(int, rating);
    PROPERTY(bool, ladder);
    PROPERTY(bool, showteam);
    PROPERTY(QString, tier);
    PROPERTY(quint16, avatar);
    PROPERTY(QString, defaultTier);
    PROPERTY(QColor, color);
    PROPERTY(bool, battleSearch);
    PROPERTY(QString, winningMessage);
    PROPERTY(QString, losingMessage);
    PROPERTY(QString, lastFindBattleIp);
public:

    QSet<int> battlesSpectated;

    enum State
    {
        NotLoggedIn=0,
        LoggedIn=1,
        Battling=2,
        Away = 4
    };

    Player(const GenericSocket &sock, int id);
    ~Player();

    /* returns all the regular info */
    TeamBattle &team();
    const TeamBattle &team() const;
    /* Converts the content of the TeamInfo to a basicInfo and returns it */
    BasicInfo basicInfo() const;

    /* Sends a message to the player */
    void sendMessage(const QString &mess, bool html=false);
    void sendChanMessage(int channel, const QString &mess, bool html=false);

    bool hasSentCommand(int commandid) const;

    int id() const;
    QString name() const;
    QString ip() const;
    QString proxyIp() const;
    int gen() const;

    bool connected() const;
    bool isLoggedIn() const;
    bool battling() const;
    void acquireKnowledgeOf(Player *other);
    void acquireRoughKnowledgeOf(Player *other);
    void addChannel(int chanid);
    void removeChannel(int chanid);
    bool hasKnowledgeOf(Player *other) const;
    bool isInSameChannel(const Player *other) const;
    bool hasBattle(int battleId) const;
    void addBattle(int battleid);
    void removeBattle(int battleid);
    bool away() const;
    bool inSearchForBattle() const { return battleSearch(); }
    void cancelBattleSearch();
    void changeState(int newstate, bool on);
    int state() const;
    int auth() const;
    void setAuth (int newAuth);
    void setName (const QString & newName);
    void setInfo(const QString &newInfo);
    const QSet<int> & getBattles() const {
        return battles;
    }

    bool okForChallenge(int src) const;
    void addChallenge(Challenge *c, bool isChallenged);
    void removeChallenge(Challenge *c);
    void cancelChallenges();
    bool okForBattle() const;
    void spectateBattle(int battleId, const BattleConfiguration &battle);
    void sendChallengeStuff(const ChallengeInfo &c);

    QSet<int> &getChannels() {
        return channels;
    }

    void doWhenDC();

    ChallengeInfo getChallengeInfo(int id); /* to get the battle info of a challenge received by that player */

    void startBattle(int battleid, int id, const TeamBattle &team, const BattleConfiguration &conf);
    void battleResult(int battleid, int result, int winner, int loser);

    void kick();

    Analyzer& relay();
    const Analyzer& relay() const;

    PlayerInfo bundle() const;

    /* A locked player will stop processing its events */
    void lock();
    void unlock();
    bool isLocked() const;
    void findTierAndRating();
    void findRating();

    void executeTierChange(const QString&);
    void executeAwayChange(bool away);
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, int chanid, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id, const QString &name);
    void sendChallenge(int source, int dest, const ChallengeInfo &desc);
    void battleFinished(int pubid, int desc, int winner, int loser);
    void updated(int id);

    void battleMessage(int battleid,int id,const BattleChoice &b);
    void battleChat(int battleid, int id, const QString &);
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
    void unlocked();
    void joinRequested(int id, const QString &channel);
    void leaveRequested(int id, int channelid);
    void ipChangeRequested(int id, const QString &ip);
public slots:
    void loggedIn(TeamInfo &team,bool,bool, QColor);
    void recvMessage(int chan, const QString &mess);
    void recvTeam(TeamInfo &team);
    void disconnected();
    void challengeStuff(const ChallengeInfo &c);
    void battleForfeited(int id);
    void battleMessage(int id, const BattleChoice &b);
    void battleChat(int id, const QString &s);
    void registerRequest();
    void hashReceived(const QString &hash);
    void playerKick(int);
    void playerBan(int);
    void CPBan(const QString &name);
    void CPUnban(const QString &name);
    //void CPTBan(const QString &name, int time);
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
    void displayRankings();
    void tUnban(QString name);
    void testAuthentificationLoaded();
    void ratingLoaded();
    void joinRequested(const QString &channel);
    void leaveRequested(int slotid);
    void ipChangeRequested(const QString &ip);
    void autoKick();
private:
    TeamBattle myteam;
    Analyzer *myrelay;
    int lockCount;

    int myid;
    int myauth;
    QString myip;
    QString proxyip;
    bool ontologin;
    mutable int lastcommand;

    int m_state;
    TeamInfo *waiting_team;
    QString waiting_name;

    QSet<int> battles;
    QSet<Challenge*> challenged;
    QSet<Challenge*> challengedBy;

    /* Each player may know other players that are in different IRC rooms.
       For example when ones uses find battle and fights someone unrelated.
       Or when one send a PM and leaves the room, the PM should still be valid
       as long as both parties are connected, and for that users need to have
       knowledge of people in other rooms.

       Typically, a player gains knowledge of another player when:
       - A player requests knowledge of another player
       - A players PMs another player
       - Battles (watching, battling, whatever)
       - <More to be added>

       When a players A gains knowledge of player B, player B also gains knowledge of Player A.
       This is because for example i want to notify to everyone Player A logged off, i just
       look at the players Player A is knowledgeable of, + the channels player A is in, and that's
       it.

       One other thing -- knowledge lasts until one players quit the server (not the room, the server).
       The reason IRC servers don't need that concept of knowledge is because they
       rely on name, and i rely on player ID. Also, it may be good to use a different
       algorithm for player ids, like increasing a counter each time a player logs on
       and then gives that id. It will make the probability of a player logging off
       and another logging on and taking his ID smaller. Though we then should provide
       scripters an easy way of looking up all players.

       Knowledge will be also used to send / or not / info on the PMer / PMee if they are not
       in each other' knowledges. Knowledge for battles is not necessary (info could just be sent
       if the players don't know each other) but keeping in memory that they know each other
       could save bandwidth.
    */
    QSet<Player*> knowledge;

    /* The channels a player is on */
    QSet<int> channels;

    void assignTeam(TeamInfo &team);
    void assignNewColor(const QColor &c);
    bool testNameValidity(const QString &name);
    void loginSuccess();
    void changeWaitingTeam(const TeamInfo &t);
    void removeWaitingTeam();
    /* only call when sure there is one battle */
    int firstBattleId();

    void testAuthentification(const QString &name);
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
