#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testchat.h"

void TestChat::onPlayerConnected()
{
    Analyzer *analyzer = dynamic_cast<Analyzer*>(sender());

    TeamHolder holder;

    holder.name() = "ArchZombie0x";

    analyzer->login(holder, false, false);
    analyzer->sendChanMessage(0, "I am the best scripter.");
}

void TestChat::onChannelMessage(const QString &message, int, bool)
{
    if (message.startsWith("ArchZombie0x")) {
        assert(message == "ArchZombie0x: I am the best scripter.");
        accept();
    }
}
