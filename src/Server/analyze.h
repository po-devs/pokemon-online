/**
 * See network protocol here: http://wiki.pokemon-online.eu/view/Network_Protocol_v2
*/

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include <QColor>
#include "network.h"
#include "../Utilities/coreclasses.h"
#include "../PokemonInfo/networkstructs.h"

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

/* Commands to dialog with the server */
namespace NetworkServ
{
#include "../Shared/networkcommands.h"
}

/***
  WARNING! Always use deleteLater on this!

  Otherwise you may delete it when Network::onReceipt() still
  does recurive calls. Crash!
***/
class Analyzer : public QObject
{
    friend class RelayManager;

    Q_OBJECT
public:
    template<class SocketClass>
    Analyzer(const SocketClass &sock, int id, bool dummy=false);
    ~Analyzer();

    /* functions called by the server */
    void sendMessage(const QString &message, bool html = false);
    void requestLogIn();
    void sendPlayer(const PlayerInfo &p);
    void sendPlayers(const QList<PlayerInfo> &p);
    void sendBattleList(int chanid, const QHash<qint32, Battle> &battles);
    void sendChannelPlayers(int channelid, const QVector<qint32> &ids);
    void sendJoin(int channelid, int playerid);
    void sendChannelBattle(int chanid, int battleid, const Battle &battle);
    void sendLogin(const PlayerInfo &p, const QStringList&, const QByteArray &reconnectPass);
    void sendLogout(int num);
    bool isConnected() const;
    void changeIP(const QString &ip);
    QString ip() const;
    void engageBattle(int battleid, int myid, int id, const TeamBattle &team, const BattleConfiguration &conf, QString tier);
    void spectateBattle(int battleid, const BattleConfiguration &conf);
    void sendBattleResult(qint32 battleid, quint8 res, quint8 mode, int win, int los);
    void sendBattleCommand(qint32 battleid, const QByteArray &command);
    void sendWatchingCommand(qint32 id, const QByteArray &command);
    void sendPM(int dest, const QString &mess);
    void sendUserInfo(const UserInfo &ui);
    void notifyBattle(qint32 battleid, qint32 id1, qint32 id2, quint8 mode, QString tier);
    void finishSpectating(qint32 battleId);
    void notifyOptionsChange(qint32 id, bool away, bool ladder);
    void startRankings(int page, int startingRank, int total);
    void sendRanking(const QString name, int points);
    void stopReceiving();
    void connectTo(const QString &host, quint16 port);
    void setLowDelay(bool lowDelay);
    void sendPacket(const QByteArray &packet);
    void sendChallengeStuff(const ChallengeInfo &c);
    void sendTeam(const QString *name, const QStringList &tierList);

    /* Closes the connection */
    void close();

    void delay();

    void swapIds(Analyzer *other);
    void setId(int id);

    /* Convenience functions to avoid writing a new one every time */
    inline void emitCommand(const QByteArray &command) {
        emit sendCommand(command);
    }

    inline bool isInCommand() const {
        return mIsInCommand;
    }

    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emitCommand(tosend);
    }
    template<class T>
    void notify_expand(int command, const T &paramList);
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    void packetToSend(const QByteArray &packet);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(LoginInfo *info);
    void serverPasswordSent(const QByteArray &hash);
    void messageReceived(int chanid, const QString &mess);
    void playerDataRequested(int playerid);
    void teamChanged(const ChangeTeamInfo&);
    void connected();
    void disconnected();
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
    void banRequested(const QString &name);
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
    void joinRequested(const QString &channel);
    void leaveChannel(int id);
    void ipChangeRequested(const QString &ip);
    void logout();
    void reconnect(int, const QByteArray&);
    /* Used to tell the command is finished - and that any pending updated() is good to go */
    void endCommand();
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);
    void undelay();
    void keepAlive();
private:
    GenericNetwork &socket();
    const GenericNetwork &socket() const;

    void dealWithCommand(const QByteArray &command);

    QLinkedList<QByteArray> delayedCommands;
    int delayCount;

    GenericNetwork *mysocket;
    QMutex mutex;
    quint16 pingedBack;
    quint16 pingSent;
    bool mIsInCommand;
    /* Is it a dummy analyzer ?*/
    bool dummy;

    ProtocolVersion version;
};

template<class SocketClass>
Analyzer::Analyzer(const SocketClass &sock, int id, bool dummy) : mysocket(new Network<SocketClass>(sock, id)), pingedBack(0), pingSent(0), mIsInCommand(false), dummy(dummy)
{
    socket().setParent(this);
    delayCount = 0;

    if (dummy) {
        return;
    }

    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(this, SIGNAL(packetToSend(QByteArray)), &socket(), SLOT(sendPacket(QByteArray)));


    QTimer *t = new QTimer(this);
    t->setInterval(30*1000);
    t->start();
    connect(t, SIGNAL(timeout()),SLOT(keepAlive()));
    /* Only if its not registry */
    if (id != 0) {
#ifndef SFML_SOCKETS
        sock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
#endif
    }
}


template<class T>
void Analyzer::notify_expand(int command, const T& paramList)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    out << uchar(command);

    typename T::const_iterator it = paramList.begin();

    while (it != paramList.end()) {
        out << *it;
        ++it;
    }

    emit sendCommand(tosend);
}


#endif // ANALYZE_H
