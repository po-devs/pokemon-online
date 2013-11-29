#include "battleservertest.h"

BattleServerTest::BattleServerTest()
{
}

void BattleServerTest::start()
{
    run();
}

void BattleServerTest::run()
{
    QTcpSocket * s = new QTcpSocket(nullptr);
    s->connectToHost("localhost", 5096);

    connect(s, SIGNAL(connected()), this, SLOT(onBattleServerConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(reject()));

    analyzer = new BaseAnalyzer(s,0);

    connect(this, SIGNAL(destroyed()), analyzer, SLOT(deleteLater()));

    /* To do: start tests on battleConnected */
}

void BattleServerTest::onBattleServerConnected()
{
    /* Default behavior for a battle server test. Override to change! */
    accept();
}
