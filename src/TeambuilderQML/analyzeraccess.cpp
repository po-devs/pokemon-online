#include "analyzeraccess.h"
#include "libraries/PokemonInfo/battlestructs.h"
#include "../Teambuilder/analyze.h"

AnalyzerAccess::AnalyzerAccess(QObject *parent) :
    QObject(parent)
{
    m_analyzer = new Analyzer();
    connect(m_analyzer, SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(m_analyzer, SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(m_analyzer, SIGNAL(connected()), SLOT(connected()));
    connect(m_analyzer, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(m_analyzer, SIGNAL(messageReceived(QString)), SLOT(printLine(QString)));
    connect(m_analyzer, SIGNAL(htmlMessageReceived(QString)), SLOT(printHtml(QString)));
    connect(m_analyzer, SIGNAL(channelMessageReceived(QString,int,bool)), SLOT(printChannelMessage(QString, int, bool)));
    connect(m_analyzer, SIGNAL(playerReceived(PlayerInfo)), SLOT(playerReceived(PlayerInfo)));
    connect(m_analyzer, SIGNAL(playerLogin(PlayerInfo, QStringList)), SLOT(playerLogin(PlayerInfo, QStringList)));
    connect(m_analyzer, SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(m_analyzer, SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(m_analyzer, SIGNAL(battleStarted(int, Battle, TeamBattle, BattleConfiguration)),
            SLOT(battleStarted(int, Battle, TeamBattle, BattleConfiguration)));
    connect(m_analyzer, SIGNAL(teamApproved(QStringList)), SLOT(tiersReceived(QStringList)));
    connect(m_analyzer, SIGNAL(battleStarted(int, Battle)), SLOT(battleStarted(int, Battle)));
    connect(m_analyzer, SIGNAL(battleFinished(int, int,int,int)), SLOT(battleFinished(int, int,int,int)));
    connect(m_analyzer, SIGNAL(battleMessage(int, QByteArray)), this, SLOT(battleCommand(int, QByteArray)));
    connect(m_analyzer, SIGNAL(passRequired(QByteArray)), SLOT(askForPass(QByteArray)));
    connect(m_analyzer, SIGNAL(serverPassRequired(QByteArray)), SLOT(serverPass(QByteArray)));
    //connect(m_analyzer, SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(m_analyzer, SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(m_analyzer, SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
    connect(m_analyzer, SIGNAL(playerTempBanned(int,int,int)), SLOT(playerTempBanned(int,int,int)));
    connect(m_analyzer, SIGNAL(PMReceived(int,QString)), SLOT(PMReceived(int,QString)));
    connect(m_analyzer, SIGNAL(awayChanged(int, bool)), SLOT(awayChanged(int, bool)));
    connect(m_analyzer, SIGNAL(ladderChanged(int,bool)), SLOT(ladderChanged(int,bool)));
    connect(m_analyzer, SIGNAL(spectatedBattle(int,BattleConfiguration)), SLOT(watchBattle(int,BattleConfiguration)));
    connect(m_analyzer, SIGNAL(spectatingBattleMessage(int,QByteArray)), SLOT(spectatingBattleMessage(int , QByteArray)));
    connect(m_analyzer, SIGNAL(spectatingBattleFinished(int)), SLOT(stopWatching(int)));
    connect(m_analyzer, SIGNAL(versionDiff(ProtocolVersion, int)), SLOT(versionDiff(ProtocolVersion, int)));
    connect(m_analyzer, SIGNAL(serverNameReceived(QString)), SLOT(serverNameReceived(QString)));
    connect(m_analyzer, SIGNAL(tierListReceived(QByteArray)), SLOT(tierListReceived(QByteArray)));
    connect(m_analyzer, SIGNAL(announcement(QString)), SLOT(announcementReceived(QString)));
    connect(m_analyzer, SIGNAL(channelsListReceived(QHash<qint32,QString>)), SLOT(channelsListReceived(QHash<qint32,QString>)));
    connect(m_analyzer, SIGNAL(channelPlayers(int,QVector<qint32>)), SLOT(channelPlayers(int,QVector<qint32>)));
    connect(m_analyzer, SIGNAL(channelCommandReceived(int,int,DataStream*)), SLOT(channelCommandReceived(int,int,DataStream*)));
    connect(m_analyzer, SIGNAL(addChannel(QString,int)), SLOT(addChannel(QString,int)));
    connect(m_analyzer, SIGNAL(removeChannel(int)), SLOT(removeChannel(int)));
    connect(m_analyzer, SIGNAL(channelNameChanged(int,QString)), SLOT(channelNameChanged(int,QString)));
    connect(m_analyzer, SIGNAL(reconnectPassGiven(QByteArray)), SLOT(setReconnectPass(QByteArray)));
    connect(m_analyzer, SIGNAL(reconnectSuccess()), SLOT(cleanData()));
    connect(m_analyzer, SIGNAL(reconnectFailure(int)), SLOT(onReconnectFailure(int)));
}

void AnalyzerAccess::connectTo(QString host, int port)
{
    m_analyzer->connectTo(host, port);
}

void AnalyzerAccess::errorFromNetwork(int a, QString b)
{
    qDebug() << "TODO AnalyzerAccess::errorFromNetwork" << a << b;
}

void AnalyzerAccess::connected()
{
    qDebug() << "TODO AnalyzerAccess::connected";
}

void AnalyzerAccess::disconnected()
{
    qDebug() << "TODO AnalyzerAccess::disconnected";
}

void AnalyzerAccess::printLine(QString s)
{
    qDebug() << "TODO AnalyzerAccess::printLine" << s;
}

void AnalyzerAccess::printHtml(QString s) {
    qDebug() << "TODO AnalyzerAccess::printHtml" << s;
}

void AnalyzerAccess::printChannelMessage(QString s, int a, bool b)
{
    qDebug() << "TODO AnalyzerAccess::printChannelMessage" << s << a << b;
}

void AnalyzerAccess::playerReceived(PlayerInfo pi)
{
    qDebug() << "TODO AnalyzerAccess::playerReceived" << pi.name;
}

void AnalyzerAccess::playerLogin(PlayerInfo pi, QStringList sl)
{
    qDebug() << "TODO AnalyzerAccess::playerLogin" << pi.name << sl;
}

void AnalyzerAccess::playerLogout(int a)
{
    qDebug() << "TODO AnalyzerAccess::playerLogout" << a;
}

void AnalyzerAccess::challengeStuff(ChallengeInfo ci)
{
    qDebug() << "TODO AnalyzerAccess::challengeStuff";
}

void AnalyzerAccess::battleStarted(int a, Battle b, TeamBattle tb, BattleConfiguration bc)
{
    qDebug() << "TODO AnalyzerAccess::battleStarted" << a;
}

void AnalyzerAccess::tiersReceived(QStringList sl)
{
    qDebug() << "TODO AnalyzerAccess::tiersReceived" << sl;
}

void AnalyzerAccess::battleStarted(int a, Battle b)
{
    qDebug() << "TODO AnalyzerAccess::battleStarted" << a;
}

void AnalyzerAccess::battleFinished(int a, int b, int c, int d)
{
    qDebug() << "TODO AnalyzerAccess::battleFinished" << a << b << c << d;
}

void AnalyzerAccess::battleCommand(int a, QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::battleCommand" << a << QString::fromLatin1(ba);

}

void AnalyzerAccess::askForPass(QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::askForPass" << QString::fromLatin1(ba);

}

void AnalyzerAccess::serverPass(QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::serverPass" << QString::fromLatin1(ba);

}

void AnalyzerAccess::setEnabled(bool b)
{
    qDebug() << "TODO AnalyzerAccess::setEnabled" << b;

}

void AnalyzerAccess::playerKicked(int a, int b)
{
    qDebug() << "TODO AnalyzerAccess::playerKicked" << a << b;

}

void AnalyzerAccess::playerBanned(int a, int b)
{
    qDebug() << "TODO AnalyzerAccess::playerBanned" << a << b;

}

void AnalyzerAccess::playerTempBanned(int a, int b, int c)
{
    qDebug() << "TODO AnalyzerAccess::playerTempBanned" << a << b << c;

}

void AnalyzerAccess::PMReceived(int a, QString s)
{
    qDebug() << "TODO AnalyzerAccess::PMReceived" << a << s;

}

void AnalyzerAccess::awayChanged(int a, bool b)
{
    qDebug() << "TODO AnalyzerAccess::awayChanged" << a << b;

}

void AnalyzerAccess::ladderChanged(int a, bool b)
{
    qDebug() << "TODO AnalyzerAccess::ladderChanged" << a << b;

}

void AnalyzerAccess::watchBattle(int a, BattleConfiguration bc)
{
    qDebug() << "TODO AnalyzerAccess::watchBattle" << a;

}

void AnalyzerAccess::spectatingBattleMessage(int a, QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::spectatingBattleMessage" << a << QString::fromLatin1(ba);

}

void AnalyzerAccess::stopWatching(int a)
{
    qDebug() << "TODO AnalyzerAccess::stopWatching" << a;

}

void AnalyzerAccess::versionDiff(ProtocolVersion pv, int a)
{
    qDebug() << "TODO AnalyzerAccess::versionDiff" << pv.version << a;

}

void AnalyzerAccess::serverNameReceived(QString s)
{
    qDebug() << "TODO AnalyzerAccess::serverNameReceived" << s;

}

void AnalyzerAccess::tierListReceived(QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::tierListReceived" << QString::fromLatin1(ba);

}

void AnalyzerAccess::announcementReceived(QString s)
{
    qDebug() << "TODO AnalyzerAccess::announcementReceived" << s;

}

void AnalyzerAccess::channelsListReceived(QHash<qint32, QString> h)
{
    qDebug() << "TODO AnalyzerAccess::channelsListReceived" << h;

}

void AnalyzerAccess::channelPlayers(int a, QVector<qint32> v)
{
    qDebug() << "TODO AnalyzerAccess::channelsListReceived" << a << v.size();

}

void AnalyzerAccess::channelCommandReceived(int a, int b, DataStream * bs)
{
    qDebug() << "TODO AnalyzerAccess::channelCommandReceived" << a << b;

}

void AnalyzerAccess::addChannel(QString s, int a)
{
    qDebug() << "TODO AnalyzerAccess::addChannel" << s << a;

}

void AnalyzerAccess::removeChannel(int a)
{
    qDebug() << "TODO AnalyzerAccess::removeChannel" << a;

}

void AnalyzerAccess::channelNameChanged(int a, QString s)
{
    qDebug() << "TODO AnalyzerAccess::channelNameChanged" << a << s;

}

void AnalyzerAccess::setReconnectPass(QByteArray ba)
{
    qDebug() << "TODO AnalyzerAccess::setReconnectPass" << QString::fromLatin1(ba);

}

void AnalyzerAccess::cleanData()
{
    qDebug() << "TODO AnalyzerAccess::cleanData";

}

void AnalyzerAccess::onReconnectFailure(int a)
{

    qDebug() << "TODO AnalyzerAccess::onReconnectFailure" << a;
}



