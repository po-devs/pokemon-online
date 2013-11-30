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

    analyzer = new BattleAnalyzer(s);

    connect(this, SIGNAL(destroyed()), analyzer, SLOT(deleteLater()));

    /* To do: start tests on battleConnected */
}

void BattleServerTest::onBattleServerConnected()
{
    connect(analyzer, SIGNAL(battleMessage(int,int,QByteArray)), SLOT(onBattleMessage(int,int,QByteArray)));
    connect(analyzer, SIGNAL(sendBattleInfos(int,int,int,TeamBattle,BattleConfiguration,QString)),
            SLOT(onBattleCreated(int,int,int,TeamBattle,BattleConfiguration,QString)));
}

void BattleServerTest::onBattleMessage(int, int, const QByteArray &)
{

}

void BattleServerTest::onBattleCreated(int, int, int, const TeamBattle &, const BattleConfiguration &, const QString &)
{

}
