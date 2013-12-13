/**
 * See network protocol here: http://wiki.pokemon-online.eu/view/Network_Protocol_v2
*/

#include "analyze.h"
#include "../Utilities/network.h"
#include "player.h"
#include "tiermachine.h"
#include "tier.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkServ;

void Analyzer::keepAlive()
{
    pingSent++;

    // You could uncomment the following, but what's the point of disconnecting someone when he could resume his connection?
//    /* If the player hasn't answered for too long, disconnecting */
//    if (pingSent - pingedBack > 9) {
//        emit logout();
//    }

    /* Seems that the keep alive option doesn't work on all computers */
    notify(KeepAlive, pingSent);
}

void Analyzer::sendMessage(const QString &message, bool html)
{
    notify(SendChatMessage, Flags(0), Flags(html==true), message);
}

void Analyzer::engageBattle(int battleid, int myid, int id, const TeamBattle &team, const BattleConfiguration &conf, const QString &tier)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly, version.version);

    out << uchar(EngageBattle); ;
    if (version < ProtocolVersion(2,0)) {
        out << qint32(battleid) << Flags(1 + (team.items.empty() ? 0 : 2)) << conf.mode;
    } else {
        out << Flags(1 + (team.items.empty() ? 0 : 2)) << qint32(battleid);
    }
    out << Battle(myid, id, conf.mode, tier);
    if (version < ProtocolVersion(1,0)) {
        conf.oldSerialize(out);
    } else {
        out << conf;
    }
    out << team;

    if (!team.items.empty()) {
        out << team.items;
    }
    emit sendCommand(tosend);
}

void Analyzer::spectateBattle(int battleid, const BattleConfiguration &conf)
{
    if (version < ProtocolVersion(1,0)) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out << uchar(SpectateBattle) << Flags(1) << qint32(battleid);
        conf.oldSerialize(out);

        emit sendCommand(tosend);
    } else {
        notify(SpectateBattle, Flags(1), qint32(battleid), conf);
    }
}

void Analyzer::sendPlayers(const QList<PlayerInfo> &p)
{
    notify_expand(PlayersList, p);
}

void Analyzer::sendPlayer(const PlayerInfo &p)
{
    notify(PlayersList, p);
}

void Analyzer::sendPM(int dest, const QString &mess)
{
    notify(SendPM, qint32(dest), mess);
}

void Analyzer::sendLogin(const PlayerInfo &p, const QStringList &tiers, const QByteArray &reconnectPass)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    out << uchar(Login) << Flags((!reconnectPass.isEmpty()) << LoginCommand::HasReconnectPass);

    if (!reconnectPass.isEmpty()) {
        out << reconnectPass;
    }

    out << p << tiers;

    emit sendCommand(tosend);
}

void Analyzer::sendRankings(quint32 id, const QHash<QString, quint32> &rankings, const QHash<QString, quint16> &ratings)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    out << uchar(ShowRankings2) << quint8(0) << quint32(id) << quint8(rankings.count());

    foreach(QString tier, rankings.keys()) {
        out << tier << ratings.value(tier) << rankings.value(tier) << quint32(TierMachine::obj()->tier(tier).count());
    }

    emit sendCommand(tosend);
}

void Analyzer::notifyOptionsChange(qint32 id, bool away, bool ladder)
{
    notify(OptionsChange, id, Flags(ladder + (away << 1)));
}

void Analyzer::sendLogout(int num)
{
    notify(Logout, qint32(num));
}

void Analyzer::sendChallengeStuff(const ChallengeInfo &c)
{
    notify(ChallengeStuff, c);
}

void Analyzer::sendBattleResult(qint32 battleid, quint8 res, quint8 mode, int winner, int loser)
{
    notify(BattleFinished, battleid, res, mode, qint32(winner), qint32(loser));
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

void Analyzer::notifyBattle(qint32 battleid, const Battle &battle)
{
    if (version < ProtocolVersion(2,0)) {
        notify(EngageBattle, battleid, Flags(0), battle.mode, battle);
    } else {
        notify(EngageBattle, Flags(0), battleid, battle);
    }
}

void Analyzer::sendUserInfo(const UserInfo &ui)
{
    notify(GetUserInfo, ui);
}

void Analyzer::finishSpectating(qint32 battleId)
{
    notify(SpectateBattle, Flags(0), battleId);
}

void Analyzer::startRankings(int page, int startingRank, int total)
{
    notify(ShowRankings, true, qint32(page), qint32(startingRank), qint32(total));
}

void Analyzer::sendRanking(QString name, int points)
{
    notify(ShowRankings, false, name, qint32(points));
}

void Analyzer::sendTeam(const QString *name, const QStringList &tierList)
{
    Flags network(0+2);
    if (name) {
        network.setFlag(0, true);
        notify(SendTeam, network, *name, tierList);
    } else {
        notify(SendTeam, network, tierList);
    }
}

void Analyzer::dealWithCommand(const QByteArray &commandline)
{
    mIsInCommand = true;

    DataStream in (commandline, version.version);
    uchar command;

    in >> command;

    switch (command) {
    case Login:
        {
            if (socket().id() != 0) {
                LoginInfo *info = new LoginInfo();
                in >> *info;

                version = info->version;

                emit loggedIn(info);
            } else
                emit accepted(); // for registry;

            break;
        }
    case Logout:
    {
        if (socket().id() == 0) {
            emit ipRefused();
        } else {
            emit logout();
        }
        break;
    }
    case NetworkServ::Reconnect:
    {
        quint32 id;
        QByteArray hash;

        in >> id >> hash;

        emit reconnect(id, hash);
        break;
    }
    case SendChatMessage:
        {
            Flags network, data;
            qint32 chanid;
            QString mess;
            in >> network >> data >> chanid >> mess;
            emit messageReceived(chanid, mess);
            break;
        }
    case PlayersList:
    {
        qint32 pid;
        in >> pid;
        emit playerDataRequested(pid);
        break;
    }
    case SendTeam:
        {
            Flags network;
            in >> network;

            ChangeTeamInfo cinfo;

            int i = 0;

#define mkptr(type, var) type var; if (network[i]) {in >> var; cinfo.var = &var;} i++;
            mkptr(QString, name);
            mkptr(QColor, color);
            mkptr(TrainerInfo, info);

            if (network[i++]) {
                bool oldTeams;
                in >> oldTeams;
                quint8 num;
                in >> num;

                if (!oldTeams) {
                    num = num > 6 ? 6 : num;

                    QList<PersonalTeam> teams;

                    for (int i = 0; i < num; i++) {
                        PersonalTeam t;
                        in >> t;
                        teams.push_back(t);
                    }

                    cinfo.teams = &teams;

                    /* Can't break out of scope, since it has a pointer to teams */
                    emit teamChanged(cinfo);
                } else {
                    PersonalTeam t;
                    in >> t;

                    cinfo.teamNum = num;
                    cinfo.team = &t;

                    emit teamChanged(cinfo);
                }
            } else {
                emit teamChanged(cinfo);
            }
#undef mkptr

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
    {
        in >> pingedBack;
        break;
    }
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
            QByteArray hash;
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
    case ServNameChange:
        if (socket().id() == 0) {
            emit invalidName();
        }
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
    case OptionsChange:
        {
            Flags f;
            in >> f;

            emit ladderChange(f[0]);
            emit awayChange(f[1]);

            break;
        }
    case CPTBan: // For compatability, v.2.0.05 uses this, now obsolete
    case CPBan:
        {
            QString name;
            qint32 time;
            in  >> name >> time;
            emit banRequested(name, time);
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
            Flags data;
            in >> id >> data;

            if (data[0])
                emit battleSpectateRequested(id);
            else
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
    case TierSelection:
        {
            quint8 team;
            QString tier;
            while (!in.atEnd()) {
                in >> team >> tier;
            }
            emit tierChanged(team, tier);
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
    case ShowRankings2:
        {
            quint8 mode;
            quint32 id;
            in >> mode >> id;
            emit showRankings(id);
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
            QByteArray hash;
            in >> hash;
            emit serverPasswordSent(hash);
            break;
        }
    default:
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
        break;
    }
    mIsInCommand = false;
    emit endCommand();
}
