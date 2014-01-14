#ifndef BATTLELOGS_H
#define BATTLELOGS_H

#include "BattleLogs_global.h"
#include "../BattleServer/plugininterface.h"
#include "../BattleServer/battleinterface.h"
#include <BattleManager/defaulttheme.h>
#include <BattleManager/battledatatypes.h>
#include <PokemonInfo/battlestructs.h>
#include <Utilities/coreclasses.h>
#include <QtCore>
#include <QWidget>

class QCheckBox;
class QTextEdit;

/*
 Saves logs in raw or plain text.

 Format for raw:

 V0-
 battle_logs_v0 <type of saving on a byte: '0' is both fully revealed, '1' is player 1, '2' is player 2, other is spectator><line break>
 <size of team1 (32 bits)><team1>
 <size of team2 (32 bits)><team2>
 <timestamp (32 bits)><command (byteArray)> * infinite

 V1-
 battle_logs_v1<line break>
 <configuration (FullBattleConfiguration)>
 <timestamp (32 bits)><command (byteArray)> * infinite

 The commands are either from a spectator point of view, a player's point of view, or both player's
 point of views (in which case spectator commands are ignored).

 BattleConfiguration helps: it holds the roles of the different players, aka Spectator or Player.

 V2-
 Same as v1, except using new serialization scheme (in which for example string is an UTF8 bytearray)

 V3-
 Now putting nature in PokeBattle before happiness (wasn't serialized before)
 Current version output: V3
*/

extern "C" {
BATTLELOGSSHARED_EXPORT BattleServerPlugin * createBattleServerPlugin();
}

class PokeBattle;
class BattleInput;
class BattleClientLog;

class BATTLELOGSSHARED_EXPORT BattleLogs
    : public BattleServerPlugin
{
public:
    BattleLogs();
    virtual ~BattleLogs() {}

    QString pluginName() const;

    BattlePlugin *getBattlePlugin(BattleInterface*);
    bool hasConfigurationWidget() const;
    QWidget * getConfigurationWidget();

    battleserver_plugin_version()

    QSet<QString> tiers;
    bool saveMixedTiers;
    bool saveRawFiles;
    bool saveTextFiles;
};

class BATTLELOGSSHARED_EXPORT BattleLogsWidget : public QWidget
{
    Q_OBJECT
public:
    BattleLogsWidget(BattleLogs *master);

    QCheckBox *mixedTiers, *rawFile, *textFile;
    QTextEdit *tiers;
    BattleLogs *master;

public slots:
    void done();
};

class BATTLELOGSSHARED_EXPORT BattleLogsPlugin
    : public BattlePlugin
{
public:
    BattleLogsPlugin(BattleInterface *b= NULL, bool raw=true, bool text=false);
    ~BattleLogsPlugin();

    QHash<QString, Hook> getHooks();
    int emitCommand(BattleInterface &, int slot, int players, QByteArray b);
    int battleStarting(BattleInterface &);
public:
    bool started;
    int id1, id2;

    QByteArray toSend;
    DataStream commands;
    QElapsedTimer t;

    FullBattleConfiguration conf;
    BattleDefaultTheme theme;
    BattleInput *input;
    BattleClientLog *log;
    battledata_basic *data;

    TeamBattle team1, team2;

    bool raw, text;
private:
    QMutex m;
};


#endif // BATTLELOGS_H
