#include "analyze.h"
#include "network.h"
#include "player.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkServ;

Analyzer::Analyzer(QTcpSocket *sock) : mysocket(sock)
{
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
}

void Analyzer::sendMessage(const QString &message)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(SendMessage) << message;

    emit sendCommand(tosend);
}

void Analyzer::engageBattle(int id, const TeamBattle &team)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(EngageBattle) << id << team;

    emit sendCommand(tosend);
}


void Analyzer::sendPlayer(int num, const BasicInfo &team)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(PlayersList) << num << team;

    emit sendCommand(tosend);
}

void Analyzer::sendLogin(int num, const BasicInfo &team)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(Login) << num << team;

    emit sendCommand(tosend);
}

void Analyzer::sendLogout(int num)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(Logout) << num;

    emit sendCommand(tosend);
}


void Analyzer::sendChallengeStuff(quint8 stuff, int num)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(ChallengeStuff) << stuff << num;

    emit sendCommand(tosend);
}

void Analyzer::sendBattleResult(quint8 res)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(BattleFinished) << res;

    emit sendCommand(tosend);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

bool Analyzer::isConnected() const
{
    return socket().isConnected();
}


void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    switch (command) {
	case Login:
	{
	    TeamInfo team;
	    in >> team;
	    emit loggedIn(team);
	    break;
	}
	case SendMessage:
	{
	    QString mess;
	    in >> mess;
	    emit messageReceived(mess);
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
	    quint8 stuff;
	    int id;
	    in >> stuff >> id;
	    emit challengeStuff(stuff, id);
	    break;
	}
	case BattleFinished:
	    emit forfeitBattle();
	    break;
	default:
	    emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
	    break;
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

