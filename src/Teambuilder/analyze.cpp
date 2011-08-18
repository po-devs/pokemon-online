#include "analyze.h"
#include "network.h"
#include "client.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"
#include "../Shared/config.h"

#include "battlewindow.h"

using namespace NetworkCli;

Analyzer::Analyzer(bool reg_connection) : registry_socket(reg_connection)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    connect(&socket(), SIGNAL(connected()), this, SLOT(wasConnected()));
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(&socket(), SIGNAL(isFull(QByteArray)), SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));

    /* Commands that will be redirected to channels */
    channelCommands << BattleList << JoinChannel << LeaveChannel << ChannelBattle << ChannelMessage << HtmlChannel;
}

void Analyzer::login(const FullInfo &team)
{
    notify(Login, team);
}

void Analyzer::sendChallengeStuff(const ChallengeInfo &c)
{
    notify(ChallengeStuff, c);
}

void Analyzer::getUserInfo(const QString &name)
{
    notify(GetUserInfo, name);
}

void Analyzer::sendPM(int id, const QString &mess)
{
    notify(SendPM, qint32(id), mess);
}

void Analyzer::sendMessage(const QString &message)
{
    notify(SendMessage, message);
}

void Analyzer::sendChanMessage(int channelid, const QString &message)
{
    notify(ChannelMessage, qint32(channelid), message);
}

void Analyzer::sendTeam(const TrainerTeam &team)
{
    notify(SendTeam, team);
}

void Analyzer::sendBattleResult(int id, int result)
{
    notify(BattleFinished, qint32(id), qint32(result));
}

void Analyzer::battleCommand(int id, const BattleChoice &comm)
{
    notify(BattleMessage, qint32(id), comm);
}

void Analyzer::channelCommand(int command, int channelid, const QByteArray &body)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);
    out << uchar(command) << qint32(channelid);

    /* We don't know for sure that << body will do what we want (as ServerSide we don't want
       to add a layer here), so a little hack, hoping datastreams concatenate data the same way as
        we do. */
    emit sendCommand(tosend + body);
}

void Analyzer::battleMessage(int id, const QString &str)
{
    if (dynamic_cast<BattleWindow*>(sender()) != NULL)
        notify(BattleChat, qint32(id), str);
    else
        notify(SpectatingBattleChat, qint32(id), str);
}

void Analyzer::CPUnban(const QString &name)
{
    notify(NetworkCli::CPUnban, name);
}

void Analyzer::CPTUnban(const QString &name)
{
    notify(NetworkCli::CPUnban, name);
}

void Analyzer::goAway(bool away)
{
    notify(Away, away);
}

void Analyzer::disconnectFromHost()
{
    socket().disconnectFromHost();
    socket().disconnect(this, SLOT(commandReceived(QByteArray)));
}

QString Analyzer::getIp() const
{
    return socket().peerAddress().toString();
}

void Analyzer::getRanking(const QString &tier, const QString &name)
{
    notify(ShowRankings, tier, false, name);
}

void Analyzer::getRanking(const QString &tier, int page)
{
    notify(ShowRankings, tier, true, qint32(page));
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    mysocket.connectToHost(host, port);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

void Analyzer::wasConnected()
{
    socket().setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    /* At least makes client use full bandwith, even if the server doesn't */
    socket().setSocketOption(QAbstractSocket::LowDelayOption, 1);
}

/*{
    QHash<qint32, Battle> battles;
    in >> battles;
    emit battleListReceived(battles);
    break;
}*/

void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    if (channelCommands.contains(command)) {
        qint32 chanid;
        in >> chanid;

        /* Because we're giving a pointer to a locally declared instance, this connection must
           be a DIRECT connection and never go in Queued mode. If you want queued mode, find another
           way to transfer the data */
        emit channelCommandReceived(command, chanid, &in);
        return;
    }
    switch (command) {
    case SendMessage: {
	    QString mess;
	    in >> mess;
	    emit messageReceived(mess);
	    break;
	}
    case HtmlMessage: {
            QString mess;
            in >> mess;
            emit htmlMessageReceived(mess);
            break;
        }
    case KeepAlive: {
            notify(KeepAlive);
            break;
        }
    case PlayersList: {
            if (!registry_socket) {
                PlayerInfo p;
                while (!in.atEnd()) {
                    in >> p;
                    emit playerReceived(p);
                }
                break;
            } else {
                // Registry socket;
                QString servName, servDesc, ip;
                quint16 numPlayers, max, port;
                in >> servName >> servDesc >> numPlayers >> ip >> max >> port;
                emit serverReceived(servName, servDesc, numPlayers, ip, max, port);
            }
	}
    case Login: {
            PlayerInfo p;
	    in >> p;
	    emit playerLogin(p);
	    break;
	}
    case Logout: {
            qint32 id;
	    in >> id;
	    emit playerLogout(id);
	    break;
	}
    case ChallengeStuff: {
            ChallengeInfo c;
            in >> c;
            emit challengeStuff(c);
	    break;
	}
    case EngageBattle: {
            qint32 battleid, id1, id2;
            in >> battleid >> id1 >> id2;

            if (id1 == 0) {
                /* This is a battle we take part in */
                TeamBattle team;
                BattleConfiguration conf;
                in >> conf >> team;
                emit battleStarted(battleid, id2, team, conf);
            } else {
                /* this is a battle of strangers */
                emit battleStarted(battleid, id1, id2);
            }
	    break;
	}
    case BattleFinished: {
            qint8 desc;
            qint32 battleid;
            qint32 id1, id2;
            in >> battleid >> desc >> id1 >> id2;
            emit battleFinished(battleid, desc, id1, id2);
	    break;
	}
    case BattleMessage: {
            qint32 battleid;
            QByteArray command;
            in >> battleid >> command;

            emit battleMessage(battleid, command);
	    break;
	}
    case AskForPass: {
            QString salt;
            in >> salt;

            if (salt.length() < 6 || strlen((" " + salt).toUtf8().data()) < 7)
                emit protocolError(5080, tr("The server requires insecure authentication."));
            emit passRequired(salt);
            break;
        }
    case Register: {
            emit notRegistered(true);
            break;
        }
    case PlayerKick: {
            qint32 p,src;
            in >> p >> src;
            emit playerKicked(p,src);
            break;
        }
    case PlayerBan: {
            qint32 p,src;
            in >> p >> src;
            emit playerBanned(p,src);
            break;
        }
    case SendTeam: {
            PlayerInfo p;
            in >> p;
            emit teamChanged(p);
            break;
        }
    case SendPM: {
            qint32 idsrc;
            QString mess;
            in >> idsrc >> mess;
            emit PMReceived(idsrc, mess);
            break;
        }
    case GetUserInfo: {
            UserInfo ui;
            in >> ui;
            emit userInfoReceived(ui);
            break;
        }
    case GetUserAlias: {
            QString s;
            in >> s;
            emit userAliasReceived(s);
            break;
        }
    case GetBanList: {
            QString s, i;
            in >> s >> i;
            emit banListReceived(s,i);
            break;
        }
    case Away: {
            qint32 id;
            bool away;
            in >> id >> away;
            emit awayChanged(id, away);
            break;
        }
    case SpectateBattle: {
            BattleConfiguration conf;
            qint32 battleId;
            in >> battleId >> conf;
            emit spectatedBattle(battleId, conf);
            break;
        }
    case SpectatingBattleMessage: {
            qint32 battleId;
            in >> battleId;
            /* Such a headache, it really looks like wasting ressources */
            char *buf;
            uint len;
            in.readBytes(buf, len);
            QByteArray command(buf, len);
            delete [] buf;
            emit spectatingBattleMessage(battleId, command);
            break;
        }
    case SpectatingBattleFinished: {
            qint32 battleId;
            in >> battleId;
            emit spectatingBattleFinished(battleId);
            break;
        }
    case VersionControl: {
            QString version;
            in >> version;
            if (version != VERSION)
                emit versionDiff(version, VERSION);
            break;
        }
    case ServerName: {
            QString serverName;
            in >> serverName;
            emit serverNameReceived(serverName);
            break;
       }
    case TierSelection: {
            QByteArray tierList;
            in >> tierList;
            emit tierListReceived(tierList);
            break;
        }
    case ShowRankings: {
            bool starting;
            in >> starting;
            if (starting)
            {
                qint32 startingPage, startingRank, total;
                in >> startingPage >> startingRank >> total;
                emit rankingStarted(startingPage, startingRank, total);
            } else {
                QString name;
                qint32 points;
                in >> name >> points;
                emit rankingReceived(name, points);
            }
            break;
        }
    case Announcement: {
            QString ann;
            in >> ann;
            emit announcement(ann);
            break;
        }
    case ChannelsList: {
            QHash<qint32, QString> channels;
            in >> channels;
            emit channelsListReceived(channels);
            break;
        }
    case ChannelPlayers: {
            QVector<qint32> ids;
            qint32 chanid;
            in >> chanid >> ids;

            emit channelPlayers(chanid, ids);
            break;
        }
    case AddChannel: {
            QString name;
            qint32 id;
            in >> name >> id;

            emit addChannel(name, id);
            break;
        }
    case RemoveChannel: {
            qint32 id;
            in >> id;

            emit removeChannel(id);
            break;
        }
    case ChanNameChange: {
            qint32 id;
            QString name;
            in >> id >> name;

            emit channelNameChanged(id, name);
            break;
        }
    case SpecialPass: {
            QSettings s;
            s.beginGroup("password");
            s.setValue(QCryptographicHash::hash(QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5), QCryptographicHash::Sha1), QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5));
            s.endGroup();
            break;
        }
    default: {
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received -- maybe an update for the program is available"));
        }
    }
}

Network & Analyzer::socket()
{
    return mysocket;
}

const Network & Analyzer::socket() const
{
    return mysocket;
}

void Analyzer::getBanList()
{
    notify(GetBanList);
}

void Analyzer::getTBanList()
{
    notify(GetTBanList);
}

void Analyzer::notify(int command)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}
