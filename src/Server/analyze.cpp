#include "analyze.h"
#include "network.h"
#include "player.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/battlestructs.h"

using namespace NetworkServ;

Analyzer::Analyzer(QTcpSocket *sock, int id) : mysocket(sock, id)
{
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));

    /* Only if its not registry */
    if (id != 0) {
        mytimer = new QTimer(this);
        connect(mytimer, SIGNAL(timeout()), this, SLOT(keepAlive()));
        mytimer->start(30000); //every 30 secs
    }
}

Analyzer::~Analyzer()
{
    blockSignals(true);
    /* Very important feature. If you don't do this it might crash.
        this makes the stillValid of Network redundant, but still.*/
    socket().close();
}

void Analyzer::sendMessage(const QString &message)
{
    notify(SendMessage, message);
}

void Analyzer::engageBattle(int , int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    notify(EngageBattle, qint32(0), qint32(id), team, conf);
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    mysocket.connectToHost(host, port);
}

void Analyzer::close() {
    socket().close();
}

QString Analyzer::ip() const {
    return socket().ip();
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

void Analyzer::keepAlive()
{
    notify(KeepAlive);
}

void Analyzer::sendChallengeStuff(const ChallengeInfo &c)
{
    notify(ChallengeStuff, c);
}

void Analyzer::sendBattleResult(quint8 res, int winner, int loser)
{
    notify(BattleFinished, res, qint32(winner), qint32(loser));
}

void Analyzer::sendBattleCommand(const QByteArray & command)
{
    notify(BattleMessage, command);
}

void Analyzer::sendWatchingCommand(qint32 id, const QByteArray &command)
{
    notify(SpectatingBattleMessage, qint32(id), command);
}

void Analyzer::notifyBattle(qint32 id1, qint32 id2)
{
    notify(EngageBattle, id1, id2);
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

void Analyzer::stopRecieving()
{
    socket().close();
}

void Analyzer::finishSpectating(qint32 battleId)
{
    notify(NetworkServ::SpectatingBattleFinished, battleId);
}


void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    qDebug() << "command received from " << socket().id() << " (" << int(command) << ")";

    switch (command) {
    case Login:
	{
            qDebug() << "Login received";
            if (mysocket.id() != 0) {
                TeamInfo team;
                bool ladder, show_team;
                QColor c;
                in >> team >> ladder >> show_team >> c;
                emit loggedIn(team,ladder,show_team,c);
            } else
                emit accepted(); // for registry;
            qDebug() << "Login end";
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
            ChallengeInfo c;
            in >> c;
            emit challengeStuff(c);
	    break;
	}
    case BattleMessage:
	{
	    BattleChoice ch;
	    in >> ch;
	    emit battleMessage(ch);
	    break;
	}
    case BattleChat:
	{
	    QString s;
	    in >> s;
	    emit battleChat(s);
	    break;
	}
    case BattleFinished:
        emit forfeitBattle();
        break;
    case KeepAlive:
        break;
    case Register:
        if (mysocket.id() != 0)
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

void Analyzer::notify(int command)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}
