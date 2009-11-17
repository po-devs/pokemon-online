#include "analyze.h"
#include "network.h"
#include "client.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkCli;

Analyzer::Analyzer()
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(&socket(), SIGNAL(isFull(QByteArray)), SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));
}

void Analyzer::login(const TrainerTeam &team)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(Login) << team;

    emit sendCommand(tosend);
}

void Analyzer::sendChallengeStuff(quint8 desc, int id)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(ChallengeStuff) << desc << id;

    emit sendCommand(tosend);
}

void Analyzer::sendMessage(const QString &message)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(SendMessage) << message;

    emit sendCommand(tosend);
}

void Analyzer::sendTeam(const TrainerTeam &team)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(SendTeam) << team;

    emit sendCommand(tosend);
}

void Analyzer::sendBattleResult(int result)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(BattleFinished) << result;

    emit sendCommand(tosend);
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
	    Player p;
	    in >> p;
	    emit playerReceived(p);
	    break;
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
	    int id;
	    in >> id;
	    emit playerLogout(id);
	    break;
	}
	case ChallengeStuff:
	{
	    quint8 stuff;
	    int id;
	    in >> stuff >> id;
	    emit challengeStuff(stuff, id);
	    break;
	}
	case EngageBattle:
	{
	    int id;
	    TeamBattle team;
	    in >> id >> team;
	    emit battleStarted(id, team);
	    break;
	}
	case BattleFinished:
	{
	    quint8 desc;
	    in >> desc;
	    emit battleFinished(desc);
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
	default:
	    emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
    }
}

Network & Analyzer::socket()
{
    return mysocket;
}
