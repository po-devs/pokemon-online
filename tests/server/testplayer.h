#ifndef TESTPLAYER_H
#define TESTPLAYER_H

#include <QObject>

#include <Teambuilder/analyze.h>

#include "test.h"

class QStringList;
class PlayerInfo;

class TestPlayer : public Test
{
    Q_OBJECT
public:
    void start();

    virtual Analyzer* sender() const;
signals:

public slots:
    virtual void onPlayerConnected(){}
    virtual void onChannelMessage(const QString &message, int chanid, bool html);
    virtual void onChannelPlayers(int chan, const QVector<qint32>& ids);
    virtual void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    virtual void playerLogout(int id);
    virtual void onPm(int player, const QString &message);
    virtual void onBattleMessage(int battle, const QByteArray &message);
protected:
    void createAnalyzer();
};

#endif // TESTPLAYER_H
