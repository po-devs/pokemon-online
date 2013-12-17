#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testsession.h"

void TestSESSION::onPlayerConnected()
{
    Analyzer *analyzer = sender();

    TeamHolder holder;
    holder.name() = "TheUnknownOne";

    analyzer->login(holder, false);
    analyzer->sendChanMessage(0, "Test: session");
    QTimer::singleShot(5000, this, SLOT(reject()));
}

void TestSESSION::onChannelMessage(const QString &message, int, bool)
{
    if (message == "fail") {
        reject();
    } else if (message == "accept") {
        accept();
    }
}
