#include "testban.h"

static Analyzer* analyzers[3];
static int pc = 0;

void TestBan::run()
{
    createAnalyzer();
    createAnalyzer();
}

void TestBan::onPlayerConnected()
{
    analyzers[pc++] = sender();

    TeamHolder holder;
    if (pc == 1) {
        holder.name() = "Vuvuzelabzz";
    } else {
        holder.name() = "trakyan";
    }

    /* Use different IPs to make sure we can ban each other */
    sender()->notify(NetworkCli::SetIP, holder.name() + QString::number(pc));
    sender()->login(holder, false);
}

void TestBan::loggedIn(const PlayerInfo &info, const QStringList &)
{
    if (pc == 2 && info.name == "trakyan") {
        analyzers[0]->tempban(info.id, 60);
    }
}

void TestBan::onPlayerDisconnected()
{
    if (pc == 2) {
        createAnalyzer();
    }
}

void TestBan::onMessage(const QString &message)
{
    if (message.contains("You are banned")) {
        accept();
    }
}
