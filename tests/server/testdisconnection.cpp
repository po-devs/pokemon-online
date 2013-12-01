#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "testdisconnection.h"

static int count = 0;
int hannah = 0;
int moogle = 0;

void TestDisconnection::onPlayerConnected()
{
    Analyzer *analyzer = sender();

    TeamHolder holder;

    if (count == 0) {
        holder.name() = "Hannah";
    } else {
        holder.name() = "Moogle";
    }

    analyzer->login(holder, false, false);

    if (++count == 1) {
        createAnalyzer();
    } else {
        /* Test shouldn't take more than that */
        QTimer::singleShot(10000, this, SLOT(reject()));
    }
}

void TestDisconnection::loggedIn(const PlayerInfo &info, const QStringList &)
{
    Analyzer *analyzer = sender();

    if (info.name == "Hannah") {
        hannah = info.id;

        if (hannah && moogle) {
            analyzer->sendPM(moogle, "<3");
        }
    } else {
        moogle = info.id;

        if (hannah && moogle) {
            analyzer->sendPM(hannah, "<3");
        }
    }
}

void TestDisconnection::playerLogout(int id)
{
    if (id == hannah || id == moogle) {
        accept();
    }
}

void TestDisconnection::onPm(int id, const QString &message)
{
    if (id != hannah && id != moogle) {
        return;
    }

    Analyzer *analyzer = sender();

    assert(message == "<3");

    if (id == hannah) {
        analyzer->sendPM(moogle, "<3");
    } else {
        analyzer->sendPM(hannah, "<3");
    }

    analyzer->disconnectFromHost();
}
