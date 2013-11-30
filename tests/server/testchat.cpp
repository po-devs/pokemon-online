#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testchat.h"

void TestChat::run()
{
    Analyzer *analyzer = new Analyzer();

    connect(this, SIGNAL(destroyed()), analyzer, SLOT(deleteLater()));
    connect(analyzer, SIGNAL(connected()), SLOT(onPlayerConnected()));
    connect(analyzer, SIGNAL(connectionError(int,QString)), SLOT(reject()));
    connect(analyzer, SIGNAL(channelMessageReceived(QString,int,bool)), SLOT(onChannelMessage(QString,int,bool)));

    analyzer->connectTo("localhost", 5080);
}

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
