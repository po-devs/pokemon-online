#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testsession.h"

void TestSession::onPlayerConnected()
{
    Analyzer *analyzer = sender();

    TeamHolder holder;
    holder.name() = "TheUnknownOne";

    analyzer->login(holder, false);
    analyzer->sendChanMessage(0, "Test: session");
    setTimeout();
}

void TestSession::onChannelMessage(const QString &message, int, bool)
{
    if (message == "fail") {
        reject();
    } else if (message == "accept") {
        accept();
    }
}
