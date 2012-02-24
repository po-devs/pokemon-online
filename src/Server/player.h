#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"
#include "playerinterface.h"
#include "sfmlsocket.h"
#include "playerstructs.h"

class Challenge;
class BattleSituation;
class Analyzer;
class ChangeTeamInfo;

/* a single player */
/***
  WARNING! Always use deleteLater!!
  This is due to Analyzer that requests a deleteLater too
***/

class Player : public QObject, public PlayerInterface
{
    Q_OBJECT

    PROPERTY(QString, winningMessage);
    PROPERTY(QString, losingMessage);
    PROPERTY(bool, battleSearch);
    PROPERTY(QString, lastFindBattleIp);
    PROPERTY(Flags, spec);
    PROPERTY(Flags, state);
public:
    enum State
    {
        LoginAttempt,
        LoggedIn,
        Battling,
        Away,
        LadderEnabled
    };

    enum Spec
    {
        First=0,
        SupportsZipCompression,
        IdsWithMessage
    };

    QSet<int> battlesSpectated;

    bool ladder() const;
    Player(const GenericSocket &sock, int id);
    ~Player();

    /* returns all the regular info */
    TeamBattle &team(int i = 0);
    const TeamBattle &team(int i = 0) const;

    /* Sends a message to the player */
    void sendLoginInfo();
    void sendMessage(const QString &mess, bool html=false);
    void sendPlayers(const QVector<reference<PlayerInfo> > & bundles);

    bool hasSentCommand(int commandid) const;

    const QString &name() const;
    QString &name();
    const QString &info() const;
    QString &info();
    const QColor &color() const;
    QColor &color();
    int id() const;
    QString ip() const;
    QString proxyIp() const;
    Pokemon::gen gen() const;
    int teamCount() const;
    int rating(const QString &tier);

    virtual const quint16& avatar() const;
    quint16 &avatar();

    bool hasTier(const QString &tier) const;
    bool connected() const;
    bool isLoggedIn() const;
    bool battling() const;
    bool supportsZip() const;
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
    int auth() const;
    void setAuth (int newAuth);
    void setName (const QString & newName);
    void setInfo(const QString &newInfo);
    const QSet<int> & getBattles() const {
        return battles;
    }
    const QSet<QString> &getTiers() const {
        return tiers;
    }

    bool okForChallenge(int src) const;
    void addChallenge(Challenge *c, bool isChallenged);
    void removeChallenge(Challenge *c);
    void cancelChallenges();
    bool okForBattle() const;
    void spectateBattle(int battleId, const BattleConfiguration &battle);
    void sendChallengeStuff(const ChallengeInfo &c);
    bool inChannel(int chan) const;

    QSet<int> &getChannels() {
        return channels;
    }

    QStringList getTierList() const;

    void doWhenDC();

    ChallengeInfo getChallengeInfo(int id); /* to get the battle info of a challenge received by that player */

    void startBattle(int battleid, int id, const TeamBattle &team, const BattleConfiguration &conf);
    void battleResult(int battleid, int result, int mode, int winner, int loser);

    void kick();

    Analyzer& relay();
    const Analyzer& relay() const;

    const PlayerInfo& bundle() const;

    /* A locked player will stop processing its events */
    void lock();
    void unlock();
    bool isLocked() const;
    void findTierAndRating(bool force=false);
    void findTier(int slot);
    void findRatings(bool force = false);
    void findRating(const QString &tier);

    void executeTierChange(int num, const QString&);
    void executeAwayChange(bool away);

    void sendPacket(const QByteArray &packet);
    void setNeedToBeUpdated(bool onlyInCommand=false);
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
    void loggedIn(LoginInfo *info);
    void serverPasswordSent(const QByteArray &hash);
    void recvMessage(int chan, const QString &mess);
    void recvTeam(const ChangeTeamInfo &info);
    void disconnected();
    void challengeStuff(const ChallengeInfo &c);
    void battleForfeited(int id);
    void battleMessage(int id, const BattleChoice &b);
    void battleChat(int id, const QString &s);
    void registerRequest();
    void hashReceived(const QByteArray &hash);
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
    void changeTier(quint8 team, const QString&);
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
    void sendUpdatedIfNeeded();
private:
    Analyzer *myrelay;
    int lockCount;

    QString myip;
    QString proxyip;
    bool ontologin;
    mutable int lastcommand;
    mutable PlayerInfo m_bundle;

    bool server_pass_sent; // XXX: maybe integrate into state? Probably needs client side things too
    /* When you want to break down in Analyzer a command in several signals, but you don't want to
      send the updated player info to every player online, use setNeedToUpdate(true) instead of
      emit updated(id). At the end of the command, the updated signal will automatically be sent */
    bool needToUpdate;

    TeamsHolder m_teams;
    QString waiting_name, waiting_pass;

    QSet<int> battles;
    QHash<QString, quint16> &ratings();
    QSet<QString> tiers;
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

    void assignNewColor(const QColor &c);
    void assignTrainerInfo(const TrainerInfo &info);
    bool testNameValidity(const QString &name);
    void loginSuccess();
    /* only call when sure there is one battle */
    int firstBattleId();
    /* called when all ratings are found */
    void ratingsFound();
    void syncTiers();
    /* Are we currently executing code directly in response to a network command received from this player ? */
    bool isInCommand() const;

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
