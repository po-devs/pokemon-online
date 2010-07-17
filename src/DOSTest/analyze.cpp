#include "analyze.h"
#include "network.h"
#include "client.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Shared/config.h"

using namespace NetworkCli;

Analyzer::Analyzer(bool reg_connection) : registry_socket(reg_connection)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(&socket(), SIGNAL(isFull(QByteArray)), SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));
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

enum ChallengeDesc
{
    Sent,
    Accepted,
    Canceled,
    Busy,
    Refused
};

void Analyzer::sendMessage(const QString &message)
{
    notify(SendMessage, message);
}

void Analyzer::sendTeam(const TrainerTeam &team)
{
    notify(SendTeam, team);
}

void Analyzer::sendBattleResult(int result)
{
    notify(BattleFinished, qint32(result));
}

void Analyzer::battleCommand(const BattleChoice &comm)
{
    notify(BattleMessage, comm);
}

void Analyzer::battleMessage(const QString &str)
{
    notify(BattleChat, str);
}

void Analyzer::battleMessage(const QString &str, int id)
{
    notify(SpectatingBattleChat, qint32(id), str);
}

void Analyzer::CPUnban(const QString &name)
{
    notify(NetworkCli::CPUnban, name);
}

void Analyzer::goAway(bool away)
{
    notify(Away, away);
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    mysocket.connectToHost(host, port);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    switch (command) {
    case SendMessage:
	{
	    QString mess;
	    in >> mess;
	    emit messageReceived(mess);
	    break;
	}
    case PlayersList:
	{
            if (!registry_socket) {
                PlayerInfo p;
                in >> p;
                emit playerReceived(p);
                break;
            } else {
                // Registry socket;
                QString servName, servDesc, ip;
                quint16 numPlayers, max;
                in >> servName >> servDesc >> numPlayers >> ip >> max;
                emit serverReceived(servName, servDesc, numPlayers, ip, max);
            }
	}
    case Login:
	{
            PlayerInfo p;
	    in >> p;
	    emit playerLogin(p);
	    break;
	}
    case Logout:
	{
            qint32 id;
	    in >> id;
	    emit playerLogout(id);
	    break;
	}
    case ChallengeStuff:
	{
            ChallengeInfo c;
            in >> c;
            emit challengeStuff(c);
	    break;
	}
    case EngageBattle:
	{
            qint32 id1, id2;
            in >> id1 >> id2;

            if (id1 == 0) {
                /* This is a battle we take part in */
                TeamBattle team;
                BattleConfiguration conf;
                in >> team >> conf;
                emit battleStarted(id2, team, conf);
            } else {
                /* this is a battle of strangers */
                emit battleStarted(id1, id2);
            }
	    break;
	}
    case BattleFinished:
	{
            qint8 desc;
	    in >> desc;
            qint32 id1, id2;
            in >> id1 >> id2;
            emit battleFinished(desc, id1, id2);
	    break;
	}
    case BattleMessage:
	{
	    /* Such a headache, it really looks like wasting ressources */
	    char *buf;
            uint len;
	    in.readBytes(buf, len);
	    QByteArray command(buf, len);
	    delete [] buf;
	    emit battleMessage(command);
	    break;
	}
    case KeepAlive:
	{
	    QByteArray tosend;
	    QDataStream out(&tosend, QIODevice::WriteOnly);
	    out.setVersion(QDataStream::Qt_4_5);

	    out << uchar(KeepAlive);

	    emit sendCommand(tosend);
	    break;
	}
    case AskForPass:
        {
            QString salt;
            in >> salt;

            if (salt.length() < 6 || strlen((" " + salt).toUtf8().data()) < 7)
                emit protocolError(5080, tr("The server requires insecure authentification."));
            emit passRequired(salt);
            break;
        }
    case Register:
        {
            emit notRegistered(true);
            break;
        }
    case PlayerKick:
        {
            qint32 p,src;
            in >> p >> src;
            emit playerKicked(p,src);
            break;
        }
    case PlayerBan:
        {
            qint32 p,src;
            in >> p >> src;
            emit playerBanned(p,src);
            break;
        }
    case SendTeam:
        {
            PlayerInfo p;
            in >> p;
            emit teamChanged(p);
            break;
        }
    case SendPM:
        {
            qint32 idsrc;
            QString mess;
            in >> idsrc >> mess;
            emit PMReceived(idsrc, mess);
            break;
        }
    case GetUserInfo:
        {
            UserInfo ui;
            in >> ui;
            emit userInfoReceived(ui);
            break;
        }
    case GetUserAlias:
        {
            QString s;
            in >> s;
            emit userAliasReceived(s);
            break;
        }
    case GetBanList:
        {
            QString s, i;
            in >> s >> i;
            emit banListReceived(s,i);
            break;
        }
    case Away:
        {
            qint32 id;
            bool away;
            in >> id >> away;
            emit awayChanged(id, away);
            break;
        }
    case SpectateBattle:
        {
            QString name0, name1;
            qint32 battleId;
            in >> name0 >> name1 >> battleId;
            emit spectatedBattle(name0, name1, battleId);
            break;
        }
    case SpectatingBattleMessage:
        {
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
    case SpectatingBattleFinished:
        {
            qint32 battleId;
            in >> battleId;
            emit spectatingBattleFinished(battleId);
            break;
        }
    case VersionControl:
        {
            QString version;
            in >> version;
            if (version != VERSION)
                emit versionDiff(version, VERSION);
            break;
        }
    case TierSelection:
        {
            QString tierList;
            in >> tierList;
            emit tierListReceived(tierList);
            break;
        }
    default:
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received -- maybe an update for the program is available"));
    }
}

Network & Analyzer::socket()
{
    return mysocket;
}

void Analyzer::getBanList()
{
    notify(GetBanList);
}

void Analyzer::notify(int command)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}
