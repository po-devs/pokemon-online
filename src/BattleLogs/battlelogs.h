#ifndef BATTLELOGS_H
#define BATTLELOGS_H

#include "BattleLogs_global.h"
#include "../Server/plugininterface.h"
#include "../Server/battleinterface.h"

#include <QtCore>
#include <QWidget>

class QCheckBox;
class QTextEdit;

/*
 Saves logs in raw or plain text.

 Format for raw:
 <version> <type of saving on a byte: 0 is both fully revealed, 1 is player 1, 2 is player 2, other is spectator><line break>
 <size of team1 (32 bits)><team1>
 <size of team2 (32 bits)><team2>
 <timestamp (32 bits)><command (8 bits)><slot (8bits)><data (byteArray)> (from each player's point of view) */

extern "C" {
BATTLELOGSSHARED_EXPORT ServerPlugin * createPluginClass(ServerInterface*);
}

class PokeBattle;

class BATTLELOGSSHARED_EXPORT BattleLogs
    : public ServerPlugin
{
public:
    BattleLogs();
    virtual ~BattleLogs() {}

    QString pluginName() const;

    BattlePlugin *getBattlePlugin(BattleInterface*);
    bool hasConfigurationWidget() const;
    QWidget * getConfigurationWidget();

    QSet<QString> tiers;
    bool saveMixedTiers;
};

class BATTLELOGSSHARED_EXPORT BattleLogsWidget : public QWidget
{
    Q_OBJECT
public:
    BattleLogsWidget(BattleLogs *master);

    QCheckBox *mixedTiers;
    QTextEdit *tiers;
    BattleLogs *master;

public slots:
    void done();
};

class BATTLELOGSSHARED_EXPORT BattleLogsPlugin
    : public BattlePlugin
{
public:
    BattleLogsPlugin();
    ~BattleLogsPlugin();

    QHash<QString, Hook> getHooks();
    int emitCommand(BattleInterface &, int slot, int players, QByteArray b);
    int battleStarting(BattleInterface &);

    bool started;
    int id1, id2;
    QByteArray teams;
    QByteArray toSend;
    QDataStream commands;
    QElapsedTimer t;
};

#endif // BATTLELOGS_H
