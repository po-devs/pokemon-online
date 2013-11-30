#ifndef TESTPLAYER_H
#define TESTPLAYER_H

#include <QObject>

#include "test.h"

class QStringList;
class PlayerInfo;

class TestPlayer : public Test
{
    Q_OBJECT
public:
    void start();

signals:

public slots:
    virtual void onPlayerConnected(){}
    virtual void onChannelMessage(const QString &message, int chanid, bool html);
    virtual void onChannelPlayers(int chan, const QVector<qint32>& ids);
    virtual void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    virtual void playerLogout(int id);
    virtual void onPm(int player, const QString &message);
protected:
    void createAnalyzer();
};

#endif // TESTPLAYER_H
