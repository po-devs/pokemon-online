/**
 * See network protocol here: https://github.com/po-devs/pokemon-online/wiki/Network-Protocol
*/

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include <Utilities/baseanalyzer.h>

class TeamBattle;
class Battle;
class BattleChoice;
class BattleConfiguration;
class ChallengeInfo;
class UserInfo;
class PlayerInfo;
class FindBattleData;
class LoginInfo;
class ChangeTeamInfo;

namespace NetworkServ {
#include "../Shared/networkcommands.h"
}

/***
  WARNING! Always use deleteLater on this!

  Otherwise you may delete it when Network::onReceipt() still
  does recurive calls. Crash!
***/
class Analyzer : public BaseAnalyzer
{
    friend class RelayManager;

    Q_OBJECT
public:
    template<class SocketClass>
    Analyzer(const SocketClass &sock, int id, bool dummy=false);

    /* functions called by the server */
    void sendMessage(const QString &message, bool html = false);
    void requestLogIn();
    void sendPlayer(const PlayerInfo &p);
    void sendPlayers(const QList<PlayerInfo> &p);
    void sendBattleList(int chanid, const QHash<qint32, Battle> &battles);
    void sendChannelPlayers(int channelid, const QVector<qint32> &ids);
    void sendJoin(int channelid, int playerid);
    void sendChannelBattle(int chanid, int battleid, const Battle &battle);
    void sendLogin(const PlayerInfo &p, const QStringList&, const QByteArray &reconnectPass, int minHTML = -1);
    void sendLogout(int num);
    void engageBattle(int battleid, int myid, int id, const TeamBattle &team, const BattleConfiguration &conf, const QString& tier);
    void spectateBattle(int battleid, const BattleConfiguration &conf);
    void sendBattleResult(qint32 battleid, quint8 res, quint8 mode, int win, int los);
    void sendBattleCommand(qint32 battleid, const QByteArray &command);
    void sendWatchingCommand(qint32 id, const QByteArray &command);
    void sendPM(int dest, const QString &mess);
    void sendUserInfo(const UserInfo &ui);
    void notifyBattle(qint32 battleid, const Battle &battle);
    void finishSpectating(qint32 battleId);
    void notifyOptionsChange(qint32 id, bool away, bool ladder);
    void startRankings(int page, int startingRank, int total);
    void sendRanking(const QString name, int points);
    void sendChallengeStuff(const ChallengeInfo &c);
    void sendTeam(const QString *name, const QStringList &tierList);
    void sendRankings(quint32 id, const QHash<QString, quint32> &rankings, const QHash<QString, quint16> &ratings);

    inline bool isInCommand() const {
        return mIsInCommand;
    }

signals:
    void loggedIn(LoginInfo *info);
    void serverPasswordSent(const QByteArray &hash);
    void messageReceived(int chanid, const QString &mess);
    void playerDataRequested(int playerid);
    void teamChanged(const ChangeTeamInfo&);
    void forfeitBattle(int id);
    void challengeStuff(const ChallengeInfo &c);
    void battleMessage(int id, const BattleChoice &choice);
    void battleChat(int id, const QString &chat);
    void battleSpectateRequested(int id);
    void battleSpectateEnded(int id);
    void battleSpectateChat(int id, const QString &chat);
    void wannaRegister();
    void sentHash(QByteArray);
    void kick(int id);
    void ban(int id);
    void banRequested(const QString &name, int time);
    void unbanRequested(const QString &name);
    void tempBan(int id, int time);
    void tempBanRequested(const QString &name, int time);
    void tunbanRequested(const QString &name);
    void PMsent(int id, const QString);
    void getUserInfo(const QString &name);
    void banListRequested();
    /* Registry socket signals */
    void ipRefused();
    void nameTaken();
    void invalidName();
    void accepted();
    void awayChange(bool away);
    void ladderChange(bool);
    void tierChanged(quint8 team, const QString &);
    void findBattle(const FindBattleData &f);
    void showRankings(const QString &tier, const QString &name);
    void showRankings(const QString &tier, int page);
    void showRankings(int id);
    void joinRequested(const QString &channel);
    void leaveChannel(int id);
    void ipChangeRequested(const QString &ip);
    void logout();
    void reconnect(int, const QByteArray&);
    /* Used to tell the command is finished - and that any pending updated() is good to go */
    void endCommand();
public slots:
    /* slots called by the network */
    void keepAlive();
protected:
    virtual void dealWithCommand(const QByteArray &command);

    quint16 pingedBack;
    quint16 pingSent;
    bool mIsInCommand;
};

template<class SocketClass>
Analyzer::Analyzer(const SocketClass &sock, int id, bool dummy) : BaseAnalyzer(sock,id,dummy), pingedBack(0), pingSent(0), mIsInCommand(false)
{
}


#endif // ANALYZE_H
