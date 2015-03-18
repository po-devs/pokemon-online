#ifndef ANALYZERACCESS_H
#define ANALYZERACCESS_H

#include "libraries/PokemonInfo/battlestructs.h"
#include "../Teambuilder/analyze.h"
#include "playerinfolistmodel.h"
#include <QObject>

class AnalyzerAccess : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel * playersInfoListModel READ playerInfoListModel NOTIFY modelChanged)
public:
    explicit AnalyzerAccess(QObject *parent = 0);
    Q_INVOKABLE void connectTo(QString host, int port);
    Q_INVOKABLE void sendChallenge(int playerId);
    Q_INVOKABLE void setPlayerName(QString name);

    QAbstractItemModel *playerInfoListModel();
signals:

    void modelChanged();
    void challengeDeclined();
public slots:
    void errorFromNetwork(int, QString);
    void connected();
    void disconnected();
    void printLine(QString);
    void printHtml(QString);
    void printChannelMessage(QString, int, bool);
    void playerReceived(PlayerInfo);
    void playerLogin(PlayerInfo, QStringList);
    void playerLogout(int);
    void challengeStuff(ChallengeInfo);
    void battleStarted(int, Battle, TeamBattle, BattleConfiguration);
    void tiersReceived(QStringList);
    void battleStarted(int, Battle);
    void battleFinished(int, int,int,int);
    void battleCommand(int, QByteArray);
    void askForPass(QByteArray);
    void serverPass(QByteArray);
    void setEnabled(bool);
    void playerKicked(int,int);
    void playerBanned(int,int);
    void playerTempBanned(int,int,int);
    void PMReceived(int,QString);
    void awayChanged(int, bool);
    void ladderChanged(int,bool);
    void watchBattle(int,BattleConfiguration);
    void spectatingBattleMessage(int , QByteArray);
    void stopWatching(int);
    void versionDiff(ProtocolVersion, int);
    void serverNameReceived(QString);
    void tierListReceived(QByteArray);
    void announcementReceived(QString);
    void channelsListReceived(QHash<qint32,QString>);
    void channelPlayers(int,QVector<qint32>);
    void channelCommandReceived(int,int,DataStream*);
    void addChannel(QString,int);
    void removeChannel(int);
    void channelNameChanged(int,QString);
    void setReconnectPass(QByteArray);
    void cleanData();
    void onReconnectFailure(int);
private:
    Analyzer * m_analyzer;
    PlayerInfoListModel *m_playerInfoListModel;
    TeamHolder *m_team;
};

#endif // ANALYZERACCESS_H
