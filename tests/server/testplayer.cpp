#include <Teambuilder/analyze.h>

#include "testplayer.h"

void TestPlayer::start()
{
    run();
}

void TestPlayer::run()
{
    createAnalyzer();
}

void TestPlayer::createAnalyzer()
{
    Analyzer *analyzer = new Analyzer();

    connect(this, SIGNAL(destroyed()), analyzer, SLOT(deleteLater()));
    connect(analyzer, SIGNAL(connected()), SLOT(onPlayerConnected()));
    connect(analyzer, SIGNAL(disconnected()), SLOT(onPlayerDisconnected()));
    connect(analyzer, SIGNAL(channelMessageReceived(QString,int,bool)), SLOT(onChannelMessage(QString,int,bool)));
    connect(analyzer, SIGNAL(messageReceived(QString)), SLOT(onMessage(QString)));
    connect(analyzer, SIGNAL(channelPlayers(int,QVector<qint32>)), SLOT(onChannelPlayers(int, QVector<qint32>)));
    connect(analyzer, SIGNAL(playerLogin(PlayerInfo,QStringList)), SLOT(loggedIn(PlayerInfo,QStringList)));
    connect(analyzer, SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(analyzer, SIGNAL(PMReceived(int,QString)), SLOT(onPm(int, QString)));
    connect(analyzer, SIGNAL(battleMessage(int,QByteArray)), SLOT(onBattleMessage(int, QByteArray)));
    connect(analyzer, SIGNAL(battleStarted(int,Battle,TeamBattle,BattleConfiguration)), SLOT(onBattleStarted(int,Battle,TeamBattle,BattleConfiguration)));
    connect(analyzer, SIGNAL(passRequired(QByteArray)), SLOT(onPassRequired(QByteArray)));

    analyzer->connectTo("localhost", 5080);
}

Analyzer *TestPlayer::sender() const
{
    return dynamic_cast<Analyzer*>(Test::sender());
}

void TestPlayer::onChannelMessage(const QString &, int, bool)
{

}

void TestPlayer::onMessage(const QString &)
{

}

void TestPlayer::onChannelPlayers(int, const QVector<qint32> &)
{

}

void TestPlayer::loggedIn(const PlayerInfo &, const QStringList &)
{

}

void TestPlayer::playerLogout(int)
{

}

void TestPlayer::onPm(int, const QString &)
{

}

void TestPlayer::onBattleMessage(int, const QByteArray &)
{

}

void TestPlayer::onPassRequired(const QByteArray &)
{

}

void TestPlayer::onBattleStarted(int, const Battle &, const TeamBattle &, const BattleConfiguration &)
{

}
