#include "../Utilities/otherwidgets.h"
#include "channel.h"
#include "player.h"
#include "server.h"
#include "server.tpp"
#include "analyze.h"
#include "scriptengine.h"

QNickValidator *Channel::checker = NULL;

unsigned int qHash(const QPointer<Player> &pl)
{
    return qHash(pl.data());
}

Channel::Channel(const QString &name, int id) : m_prop_id(id), m_prop_name(name), logDay(0) {
    QDir d;
    if(!d.exists("logs/chat/" + name)) {
        d.mkpath("logs/chat/" + name);
    }
}

void Channel::log(const QString &message) {
    if(!logfile.isOpen() || logDay != QDate::currentDate().day()) {
        if(logfile.isOpen()) {
            logfile.close();
        }
        QString date = QDate::currentDate().toString("yyyy-MM-dd");
        QString filename = "logs/chat/"+name()+"/"+date+".txt";
        logDay = QDate::currentDate().day();
        logfile.setFileName(filename);
        logfile.open(QFile::WriteOnly | QFile::Append | QFile::Text);
    }
    logfile.write(QString("(%1) %2\n").arg(QTime::currentTime().toString("hh:mm:ss"), message).toUtf8());
    logfile.flush();
}

bool Channel::validName(const QString &name) {
    return checker->validate(name) == QValidator::Acceptable;
}

void Channel::addBattle(int battleid, const Battle &b)
{
    if (battleList.contains(battleid)) {
        return;
    }

    battleList[battleid] = b;
    foreach(Player *p, players) {
        p->relay().sendChannelBattle(id(), battleid, b);
    }
}

void Channel::leaveRequest(Player *player, bool onlydisconnect)
{
    Server *server = Server::serverIns;

    if (players.contains(player)) {
        server->engine()->beforeChannelLeave(player->id(), id());

        foreach(Player *p, players) {
            p->relay().notify(NetworkServ::LeaveChannel, qint32(id()), qint32(player->id()));
        }

        foreach(int battleid, player->getBattles()) {
            Battle b = server->ongoingBattle(battleid);
            /* We remove the battle only if only one of the player is in the channel */
            if (int(players.contains(server->player(b.id1))) + int(players.contains(server->player(b.id2))) < 2) {
                battleList.remove(battleid);
            }
        }

        server->printLine(QString("%1 left channel %2.").arg(player->name(), name()));
        players.remove(player);

        if (onlydisconnect) {
            disconnectedPlayers.insert(player);
        } else {
            player->removeChannel(id());
        }

        server->engine()->afterChannelLeave(player->id(), id());
    } else if (disconnectedPlayers.contains(player)) {
        disconnectedPlayers.remove(player);
        player->removeChannel(id());
    }

    if (players.size() <= 0) {
        emit closeRequest(id());
    }
}

void Channel::playerJoin(Player *player)
{
    Server *server= Server::serverIns;

    QVector<qint32> ids;
    ids.reserve(players.size());

    QSet<Player*> unknown;
    QVector<reference<PlayerInfo> > bundles;

    Analyzer &relay = player->relay();
    foreach(Player *p, players) {
        if (!p->isInSameChannel(player)) {
            unknown.insert(p);
            bundles.push_back(&p->bundle());
        }
        ids.push_back(p->id());
    }

    server->notifyGroup(unknown, NetworkServ::PlayersList, player->bundle());

    player->sendPlayers(bundles);
    relay.sendChannelPlayers(id(), ids);

    disconnectedPlayers.remove(player);
    players.insert(player);

    player->addChannel(id());

    server->printLine(QString("%1 joined channel %2.").arg(player->name(), name()));

    foreach(Player *p, players) {
        p->relay().sendJoin(player->id(), id());
    }

    relay.sendBattleList(id(), battleList);

    foreach(int battleid, player->getBattles()) {
        /* We stop showing a battle when the battle has a result ended. But it doesn't mean
         * the battle isn't in the player memory, as the player can still chat in an ended battle.
         *
         * So we need to test if the battle is ongoing or not.
         */
        if (server->hasOngoingBattle(battleid)) {
            addBattle(battleid, server->ongoingBattle(battleid));
        }
    }
}

Channel::~Channel()
{
    foreach(Player *p, players) {
        p->removeChannel(id());
    }
    foreach(Player *p, disconnectedPlayers) {
        if (p) {
            p->removeChannel(id());
        }
    }

    if(logfile.isOpen()) {
        logfile.close();
    }
}
