#include "testreconnect.h"

namespace {
    int count = 0;
    int logincount = 0;
    int success = 0;

    int id = 0;
    Analyzer *oldSender(0);
}

void TestReconnect::onPlayerConnected()
{
    setTimeout(20);

    if (++count <= 1) {
        sender()->ownVersion = ProtocolVersion(1337, 508);
        sender()->login(TeamHolder("ultimate version"), false);
    } else {
        sender()->reconnect(id, oldSender->reconnectPass, oldSender->commandCount);
    }
}

void TestReconnect::onPlayerDisconnected()
{
    /* no rejection */
}

void TestReconnect::loggedIn(const PlayerInfo &info, const QStringList &)
{
    if (++logincount <= 1) {
        oldSender = sender();
        id = info.id;
        createAnalyzer();
    }
}

void TestReconnect::onReconnectSuccess()
{
    sender()->sendChanMessage(0, "eval: '<version>: ' + sys.protocolVersion(sys.id('ultimate version'))");
}

void TestReconnect::onChannelMessage(const QString &message, int, bool)
{
    if (message.startsWith("<version>: ")) {
        assert(message.mid(11) == "1337.508");
        if (++::success == 4) {
            accept();
        } else {
            sender()->sendChanMessage(0, "eval: sys.disconnect(sys.id('ultimate version'))");
            QTimer::singleShot(5000, this, SLOT(createAnalyzer()));
        }
    }

    /* called twice */
    if (message == "ultimate version reconnected.") {
        ++::success;
    }
}
