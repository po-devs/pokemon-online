#include "analyze.h"
#include "network.h"
#include "player.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkServ;

Analyzer::~Analyzer()
{
    blockSignals(true);
    /* Very important feature. If you don't do this it might crash.
        this makes the stillValid of Network redundant, but still.*/
    socket().close();
}

void Analyzer::keepAlive()
{
    if (!pingedBack) {
        //emit disconnected();
        //return;
    }
    pingedBack = false;
    /* Seems that the keep alive option doesn't work on all computers */
    notify(KeepAlive);
}

void Analyzer::sendMessage(const QString &message)
{
    notify(SendMessage, message);
}

void Analyzer::sendChannelMessage(int chanid, const QString &message)
{
    notify(ChannelMessage, qint32(chanid), message);
}

void Analyzer::sendHtmlMessage(const QString &message)
{
    notify(HtmlMessage, message);
}

void Analyzer::sendHtmlChannelMessage(int chanid, const QString &message)
{
    notify(HtmlChannel, qint32(chanid), message);
}

void Analyzer::engageBattle(int battleid, int , int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    notify(EngageBattle, qint32(battleid), qint32(0), qint32(id), conf, team);
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    socket().connectToHost(host, port);
}

void Analyzer::close() {
    socket().close();
}

QString Analyzer::ip() const {
    return socket().ip();
}

void Analyzer::setLowDelay(bool lowDelay)
{
    socket().setLowDelay(lowDelay);
}

void Analyzer::sendPlayers(const QList<PlayerInfo> &p)
{
    notify_expand(PlayersList, p);
}

void Analyzer::sendPlayer(const PlayerInfo &p)
{
    notify(PlayersList, p);
}

void Analyzer::sendTeamChange(const PlayerInfo &p)
{
    notify(SendTeam, p);
}

void Analyzer::sendPM(int dest, const QString &mess)
{
    notify(SendPM, qint32(dest), mess);
}

void Analyzer::sendLogin(const PlayerInfo &p)
{
    notify(Login, p);
}

void Analyzer::notifyAway(qint32 id, bool away)
{
    notify(Away, id, away);
}

void Analyzer::sendLogout(int num)
{
    notify(Logout, qint32(num));
}

void Analyzer::sendChallengeStuff(const ChallengeInfo &c)
{
    notify(ChallengeStuff, c);
}

void Analyzer::sendBattleResult(qint32 battleid, quint8 res, int winner, int loser)
{
    notify(BattleFinished, battleid, res, qint32(winner), qint32(loser));
}

void Analyzer::sendBattleCommand(qint32 battleid, const QByteArray & command)
{
    notify(BattleMessage,battleid, command);
}

void Analyzer::sendWatchingCommand(qint32 id, const QByteArray &command)
{
    notify(SpectatingBattleMessage, qint32(id), command);
}

void Analyzer::sendBattleList(int channelid, const QHash<int, Battle> &battles)
{
    notify(BattleList, qint32(channelid), battles);
}

void Analyzer::sendJoin(int playerid, int channelid)
{
    notify(JoinChannel, qint32(channelid), qint32(playerid));
}

void Analyzer::sendChannelBattle(int chanid, int battleid, const Battle &battle)
{
    notify(ChannelBattle, qint32(chanid), qint32(battleid), battle);
}

void Analyzer::sendChannelPlayers(int channelid, const QVector<qint32> &ids)
{
    notify(ChannelPlayers, qint32(channelid), ids);
}

void Analyzer::notifyBattle(qint32 battleid, qint32 id1, qint32 id2)
{
    notify(EngageBattle, battleid , id1, id2);
}

void Analyzer::sendUserInfo(const UserInfo &ui)
{
    notify(GetUserInfo, ui);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

bool Analyzer::isConnected() const
{
    return socket().isConnected();
}

void Analyzer::stopReceiving()
{
    socket().close();
}

void Analyzer::finishSpectating(qint32 battleId)
{
    notify(SpectatingBattleFinished, battleId);
}

void Analyzer::startRankings(int page, int startingRank, int total)
{
    notify(ShowRankings, true, qint32(page), qint32(startingRank), qint32(total));
}

void Analyzer::sendRanking(QString name, int points)
{
    notify(ShowRankings, false, name, qint32(points));
}

void Analyzer::dealWithCommand(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    switch (command) {
    case Login:
        {
            if (socket().id() != 0) {
                TeamInfo team;
                bool ladder, show_team;
                QColor c;
                in >> team >> ladder >> show_team >> c;
                emit loggedIn(team,ladder,show_team,c);
            } else
                emit accepted(); // for registry;

            break;
        }
        /* If used, would be used to dial with the server directly */
/*    case SendMessage:
        {
            QString mess;
            in >> mess;
            emit messageReceived(mess);
            break;
        } */
    case ChannelMessage:
        {
            qint32 chanid;
            QString mess;
            in >> chanid >> mess;
            emit messageReceived(chanid, mess);
            break;
        }
    case SendTeam:
        {
            TeamInfo team;
            in >> team;
            emit teamReceived(team);
            break;
        }
    case ChallengeStuff:
        {
            ChallengeInfo c;
            in >> c;
            emit challengeStuff(c);
            break;
        }
    case BattleMessage:
        {
            qint32 id;
            in >> id;
            BattleChoice ch;
            in >> ch;
            emit battleMessage(id,ch);
            break;
        }
    case BattleChat:
        {
            qint32 id;
            in >> id;
            QString s;
            in >> s;
            emit battleChat(id,s);
            break;
        }
    case BattleFinished:
        qint32 id;
        in >> id;
        emit forfeitBattle(id);
        break;
    case KeepAlive:
        pingedBack = true;
        break;
    case Register:
        if (socket().id() != 0)
        {
            qDebug() << "Hash received";
            emit wannaRegister();
            qDebug() << "End Hash received";
        }
        else
            emit nameTaken(); // for registry
        break;
    case AskForPass:
        {
            QString hash;
            in >> hash;
            emit sentHash(hash);
            break;
        }
    case PlayerKick:
        {
            qint32 id;
            in >> id;
            emit kick(id);
            break;
        }
    case PlayerBan:
        {
            qint32 id;
            in >> id;
            emit ban(id);
            break;
        }
    case Logout:
        emit ipRefused();
        break;
    case ServNameChange:
        emit invalidName();
        break;
    case SendPM:
        {
            qint32 id;
            QString s;
            in >> id >> s;
            emit PMsent(id, s);
            break;
        }
    case GetUserInfo:
        {
            QString name;
            in >> name;
            emit getUserInfo(name);
            break;
        }
    case GetBanList:
        emit banListRequested();
        break;
    case Away:
        {
            bool away;
            in >> away;
            emit awayChange(away);
            break;
        }
    case CPBan:
        {
            QString name;
            in >> name;
            emit banRequested(name);
            break;
        }
    case CPUnban:
        {
            QString name;
            in >> name;
            emit unbanRequested(name);
            break;
        }
    case SpectateBattle:
        {
            qint32 id;
            in >> id;
            emit battleSpectateRequested(id);
            break;
        }
    case SpectatingBattleFinished:
        {
            qint32 id;
            in >> id;
            emit battleSpectateEnded(id);
            break;
        }
    case SpectatingBattleChat:
        {
            qint32 id;
            QString str;
            in >> id >> str;
            emit battleSpectateChat(id, str);
            break;
        }
    case LadderChange:
        {
            bool change;
            in >> change;
            emit ladderChange(change);
            break;
        }
    case ShowTeamChange:
        {
            bool change;
            in >> change;
            emit showTeamChange(change);
            break;
        }
    case TierSelection:
        {
            QString tier;
            in >> tier;
            emit tierChanged(tier);
            break;
        }
    case FindBattle:
        {
            FindBattleData f;
            in >> f;
            emit findBattle(f);
            break;
        }
    case ShowRankings:
        {
            bool bypage;
            QString tier;
            in >> tier >> bypage;
            if (bypage) {
                qint32 page;
                in >> page;
                emit showRankings(tier, page);
            } else {
                QString name;
                in >> name;
                emit showRankings(tier, name);
            }
            break;
        }
    case CPTBan:
        {
            QString name;
            qint32 time;
            in  >> name >> time;
            emit tempBanRequested(name, time);
            break;
        }
    case PlayerTBan:
        {
            qint32 id;
            qint32 time;
            in  >> id >> time;
            emit tempBan(id, time);
            break;
        }
    case CPTUnban:
        {
            QString name;
            in >> name;
            emit tunbanRequested(name);
            break;
        }
    case GetTBanList:
        emit tbanListRequested();
        break;
    case JoinChannel:
        {
            QString name;
            in >> name;
            emit joinRequested(name);
            break;
        }
    case LeaveChannel:
        {
            qint32 id;
            in >> id;
            emit leaveChannel(id);
            break;
        }
    case SetIP:
        {
            QString ip;
            in >> ip;
            emit ipChangeRequested(ip);
            break;
        }
    case ServerPass:
        {
            QString hash;
            in >> hash;
            emit serverPasswordSent(hash);
            break;
        }
    default:
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
        break;
    }
}

void Analyzer::commandReceived(const QByteArray &command)
{
    if (delayCount > 0) {
        delayedCommands.push_back(command);
    } else {
        dealWithCommand(command);
    }
}

void Analyzer::delay()
{
    delayCount += 1;
}

void Analyzer::undelay()
{
    delayCount -=1;

    while(delayedCommands.size() > 0 && delayCount==0) {
        dealWithCommand(delayedCommands.takeFirst());
    }
}

GenericNetwork & Analyzer::socket()
{
    return *mysocket;
}

const GenericNetwork & Analyzer::socket() const
{
    return *mysocket;
}

void Analyzer::notify(int command)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}
