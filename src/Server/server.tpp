#include "server.h"
#include "networkutilities.h"
#include "player.h"

template <typename ...Params>
void Server::notifyGroup(PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    notifyGroup(group, packet);
}

template <typename ...Params>
void Server::notifyGroup(const QSet<Player*>& group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);

    foreach(Player *p, group) {
        p->sendPacket(packet);
    }
}

template <typename ...Params>
void Server::notifyOppGroup(PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);

    const QSet<Player*> &g = getOppGroup(group);

    foreach(Player *p, g) {
        p->sendPacket(packet);
    }
}

template <typename ...Params>
void Server::notifyChannel(int channel, PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    const QSet<Player*> &g1 = getGroup(group);
    const QSet<int> &g2 = this->channel(channel).players;

    if (g1.size() < g2.size()) {
        foreach(Player *p, g1) {
            if (p->inChannel(channel)) {
                p->sendPacket(packet);
            }
        }
    } else {
        foreach(int pid, g2) {
            Player *p = player(pid);
            if (group == All || p->spec()[group]) {
                p->sendPacket(packet);
            }
        }
    }
}

template <typename ...Params>
void Server::notifyChannelOpp(int channel, PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    const QSet<Player*> &g1 = getOppGroup(group);
    const QSet<int> &g2 = this->channel(channel).players;

    if (g1.size() < g2.size()) {
        foreach(Player *p, g1) {
            if (p->inChannel(channel)) {
                p->sendPacket(packet);
            }
        }
    } else {
        foreach(int pid, g2) {
            Player *p = player(pid);
            if (!p->spec()[group]) {
                p->sendPacket(packet);
            }
        }
    }
}

template <typename ...Params>
void Server::notifyChannelLastId(int channel, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);

    foreach(int pid, this->channel(channel).players) {
        Player *p = player(pid);
        if (!p->hasSentCommand(lastDataId)) {
            p->sendPacket(packet);
        }
    }
}

template <typename ...Params>
void Server::notifyAll(int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    foreach(Player *p, myplayers) {
        p->sendPacket(packet);
    }
}
