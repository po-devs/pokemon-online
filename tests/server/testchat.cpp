#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testchat.h"

void TestChat::onPlayerConnected()
{
    sender()->login(TeamHolder("ArchZombie0x"), false);
    sender()->sendChanMessage(0, "I am the best scripter.");
}

void TestChat::onChannelMessage(const QString &message, int, bool)
{
    if (message.startsWith("ArchZombie0x")) {
        assert(message == "ArchZombie0x: I am the best scripter.");
        accept();
    }
}
