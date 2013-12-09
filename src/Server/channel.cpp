#include <cassert>
#include "../Utilities/otherwidgets.h"
#include "channel.h"
#include "player.h"
#include "server.h"
#include "server.tpp"
#include "analyze.h"
#include "scriptengine.h"

QNickValidator *Channel::checker = new QNickValidator(nullptr);

Channel::Channel(const QString &name, int id) : m_prop_id(id), m_prop_name(name){
    server = Server::serverIns;
}

void Channel::log(const QString &message) {
    (void) message;
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
    foreach(int pid, players) {
        server->player(pid)->relay().sendChannelBattle(id(), battleid, b);
    }
}

void Channel::playerJoin(int pid)
{
    Player *player = server->player(pid);

    notifyJoin(pid);

    disconnectedPlayers.remove(pid);
    players.insert(pid);

    player->addChannel(id());

    server->printLine(QString("%1 joined channel %2.").arg(player->name(), name()));

    foreach(int pid2, players) {
        server->player(pid2)->relay().sendJoin(pid, id());
    }

    addBattles(player);
}

void Channel::addDisconnectedPlayer(int pid)
{
    assert(!players.contains(pid));

    server->player(pid)->addChannel(id());
    disconnectedPlayers.insert(pid);
}

void Channel::leaveRequest(int pid)
{
    Player *player = server->player(pid);

    if (players.contains(pid)) {
        assert(!disconnectedPlayers.contains(pid));
        server->engine()->beforeChannelLeave(player->id(), id());

        notifyLeave(pid);
        removeBattles(player);

        players.remove(pid);
        player->removeChannel(id());

        server->printLine(QString("%1 left channel %2.").arg(player->name(), name()));
        server->engine()->afterChannelLeave(pid, id());
    } else {
        assert(disconnectedPlayers.contains(pid));

        disconnectedPlayers.remove(pid);
        player->removeChannel(id());
    }

    if (isEmpty()) {
        emit closeRequest(id());
    }
}

void Channel::onReconnect(int playerid)
{
    QVector<qint32> ids;
    ids.reserve(players.size());

    Player *player = server->player(playerid);

    QVector<reference<PlayerInfo> > bundles;

    Analyzer &relay = player->relay();
    foreach(int pid, players) {
        ids.push_back(pid);

        Player *p = server->player(pid);
        if (!p->isInSameChannel(player)) {
            bundles.push_back(&p->bundle());
        }
    }

    player->sendPlayers(bundles);

    relay.sendChannelPlayers(id(), ids);
    player->addChannel(id());

    relay.sendBattleList(id(), battleList);
}

void Channel::warnAboutRemoval()
{
    foreach(int p, players) {
        server->player(p)->removeChannel(id());
    }
    foreach(int p, disconnectedPlayers) {
        server->player(p)->removeChannel(id());
    }

    players.clear();
    disconnectedPlayers.clear();
}

void Channel::onRemoval()
{
    warnAboutRemoval();
    deleteLater();
}

bool Channel::isEmpty() const
{
    return players.size() == 0;
}

int Channel::count() const
{
    return players.size();
}

Channel::~Channel()
{
    warnAboutRemoval();
}


void Channel::notifyJoin(int pid)
{
    Player *player = server->player(pid);

    QVector<qint32> ids;
    ids.reserve(players.size());

    /* Players of this channel which don't already know the new player */
    QSet<Player*> unknown;
    /* The info of those players, to be sent to the player to join */
    QVector<reference<PlayerInfo> > bundles;

    Analyzer &relay = player->relay();
    foreach(int pid2, players) {
        Player *p = server->player(pid2);
        if (!p->isInSameChannel(player)) {
            unknown.insert(p);
            bundles.push_back(&p->bundle());
        }
        ids.push_back(p->id());
    }

    server->notifyGroup(unknown, NetworkServ::PlayersList, player->bundle());

    player->sendPlayers(bundles);
    relay.sendChannelPlayers(id(), ids);
}

void Channel::notifyLeave(int pid)
{
    foreach(int pid2, players) {
        server->player(pid2)->relay().notify(NetworkServ::LeaveChannel, qint32(id()), qint32(pid));
    }
}

void Channel::addBattles(Player *player)
{
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

    player->relay().sendBattleList(id(), battleList);
}

void Channel::removeBattles(Player *player)
{
    foreach(int battleid, player->getBattles()) {
        /* Because a player has a battle, it doesn't mean it's ongoing */
        if (server->hasOngoingBattle(battleid)) {
            Battle b = server->ongoingBattle(battleid);
            /* We remove the battle only if only one (or less) of the players are in the channel */
            if (int(players.contains(b.id1)) + int(players.contains(b.id2)) < 2) {
                battleList.remove(battleid);
            }
        }
    }
}
