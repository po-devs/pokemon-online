#include "analyze.h"
#include "network.h"
#include "client.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkCli;

Analyzer::Analyzer(bool reg_connection) : registry_socket(reg_connection)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(&socket(), SIGNAL(isFull(QByteArray)), SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));
}

void Analyzer::login(const TrainerTeam &team)
{
    notify(Login, team);
}

void Analyzer::sendChallengeStuff(quint8 desc, int id)
{
    notify(ChallengeStuff, desc, qint32(id));
}

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
                Player p;
                in >> p;
                emit playerReceived(p);
                break;
            } else {
                // Registry socket;
                QString servName, servDesc, ip;
                quint16 numPlayers;
                in >> servName >> servDesc >> numPlayers >> ip;
                emit serverReceived(servName, servDesc, numPlayers, ip);
            }
	}
    case Login:
	{
	    Player p;
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
	    quint8 stuff;
            qint32 id;
	    in >> stuff >> id;
	    emit challengeStuff(stuff, id);
	    break;
	}
    case EngageBattle:
	{
            qint32 id;
	    TeamBattle team;
	    BattleConfiguration conf;
	    in >> id >> team >> conf;
	    emit battleStarted(id, team, conf);
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
            Player p;
            in >> p;
            emit teamChanged(p);
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

void Analyzer::notify(int command)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}
