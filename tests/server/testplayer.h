#ifndef TESTPLAYER_H
#define TESTPLAYER_H

#include <QObject>

#include <PokemonInfo/teamholder.h>
#include <Teambuilder/analyze.h>

#include "test.h"

class QStringList;
class PlayerInfo;

class TestPlayer : public Test
{
    Q_OBJECT
public:
    void start();
    void run();

    virtual Analyzer* sender() const;
signals:

public slots:
    /* Use onPlayerConnected as the entry point of most of your tests. From then you can
     * use sender() to log in */
    virtual void onPlayerConnected(){}
    virtual void onPlayerDisconnected(){reject();}
    virtual void onChannelMessage(const QString &message, int chanid, bool html);
    virtual void onMessage(const QString&);
    virtual void onChannelPlayers(int chan, const QVector<qint32>& ids);
    virtual void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    virtual void playerLogout(int id);
    virtual void onPm(int player, const QString &message);
    virtual void onBattleMessage(int battle, const QByteArray &message);
    virtual void onPassRequired(const QByteArray &);
    virtual void onBattleStarted(int, const Battle &b, const TeamBattle &t, const BattleConfiguration &conf);
protected:
    void createAnalyzer();
};

#endif // TESTPLAYER_H
