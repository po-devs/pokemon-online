#ifndef CLIENT_H
#define CLIENT_H

#include <QtGui>
#include <QStackedWidget>
#include "analyze.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"
#include "centralwidget.h"
#include "../Utilities/otherwidgets.h"
#include "tierstruct.h"
#include "password_wallet.h"
#include "Teambuilder/teamholder.h"
#include "clientinterface.h"
#include "plugininterface.h"

class TeamBuilder;
class MainEngine;
class ChallengeDialog;
class QIdTreeWidgetItem;
class BattleWindow;
class BaseBattleWindowInterface;
class QScrollDownTextBrowser;
class PMSystem;
struct PMStruct;
class ControlPanel;
class RankingDialog;
class FindBattleDialog;
struct FindBattleData;
class Channel;
class QExposedTabWidget;
class SmallPokeTextEdit;
class DataStream;
class PluginManager;

/* The class for going online.

    It displays the mainchat, the players list, ... and also have the dialog engine in it*/

class Client : public QWidget, public ClientInterface, public CentralWidgetInterface
{
    Q_OBJECT

    friend class Channel;
public:
    Client(PluginManager*, TeamHolder *, const QString &url, const quint16 port);
    ~Client();

    TeamHolder *team();
    QMenuBar *createMenuBar(MainEngine *w);

    void addPlugin(OnlineClientPlugin *o);
    void removePlugin(OnlineClientPlugin *o);
    void registerMetaTypes(QScriptEngine *);

    /* Prints a line to all the channels which have that player */
    void printLine(int playerid, const QString &line);
    void printLine(int event, int playerid, const QString &line);

    Q_INVOKABLE void cancelFindBattle(bool verbose=true);
    Q_INVOKABLE bool playerExist(int id) const;
    Q_INVOKABLE QString name(int id) const;
    Q_INVOKABLE QString ownName() const;
    Q_INVOKABLE bool battling() const;
    Q_INVOKABLE bool busy() const;
    Q_INVOKABLE bool hasChannel(int channelid) const;
    Q_INVOKABLE bool away() const;
    Q_INVOKABLE int id(const QString &name) const;
    Q_INVOKABLE int currentChannel() const;
    Q_INVOKABLE QString channelName(int id) const;
    Q_INVOKABLE const QHash<qint32, QString>& getChannelNames() const;
    Q_INVOKABLE int channelId(const QString &name) const;
    Q_INVOKABLE Channel *channel(int channelid);
    Q_INVOKABLE int ownId() const;
    Q_INVOKABLE int ownAuth() const;

    Q_INVOKABLE int auth(int id) const;
    Q_INVOKABLE bool isIgnored(int id) const;

    Q_INVOKABLE QString authedNick(int id) const;
    Q_INVOKABLE QColor color(int id) const;

    Q_INVOKABLE QString tier(int player) const;
    Q_INVOKABLE QStringList tiers(int player) const;

    void changeName(int player, const QString &name);
    void changeChannelId(int orId, int destId);
    /* Resets fade away counter */
    void refreshPlayer(int id);

    bool hasPlayer(int id);
    bool hasPlayerInfo(int id);
    bool hasKnowledgeOf(int id);

    QSize defaultSize() const {
        return QSize(800,600);
    }
    Q_INVOKABLE void reconnect();

    Q_INVOKABLE QString defaultChannel();

    Q_INVOKABLE QString announcement();

    Q_INVOKABLE QStringList autojoinChannels();
    Q_INVOKABLE QStringList myChannels();
    enum Status {
        Available = 0,
        Away,
        Battling,
        Ignored,
        LastStatus
    };
    QIcon statusIcon(int auth, Status status) const;

    void seeChallenge(const ChallengeInfo &c);

    Q_INVOKABLE void sendChallenge(int id, int clauses, int mode);
    Q_INVOKABLE void acceptChallenge(int cId);

    PlayerInfo player(int id) const;
    void removePlayer(int id);

    void removeBattleWindow(int id);
    void disableBattleWindow(int id);

    void onDisconnection();

    QList<QIcon> statusIcons;
    QIcon chatot, greychatot;

    Q_INVOKABLE QStringList getTierList() const {
        return tierList;
    }

    void printEvent(int event, int playerid, const QString &line);
    /* Show player events, sort by tier, show timestamps */
    enum PlayerEvent {
        NoEvent = 0,
        IdleEvent = 1,
        BattleEvent = 2,
        ChannelEvent = 4,
        TeamEvent = 8,
        AnyEvent = 15
    };

    int showPEvents;
    bool sortBT;
    bool sortBA;
    bool sortCBN;
    bool showTS;
    bool pmFlashing;
    bool pmsTabbed;
    bool pmReject;

    bool flashingToggled;

    TierNode tierRoot;
    QStringList tierList;
public slots:
    void errorFromNetwork(int errnum, const QString &error);

    void connected();
    void disconnected();

    /* message received from the server */
    void printLine(const QString &line);
    void printHtml(const QString &html);
    void printChannelMessage(const QString &mess, int channel, bool html);

    void trayMessage(const QString &title, const QString &message);
    bool windowActive();

    /* sends what's in the line edit */
    void sendText();
    void changeName(const QString&);
    void playerLogin(const PlayerInfo &p, const QStringList &tiers, bool ignore=false);
    void playerReceived(const PlayerInfo &p);
    void announcementReceived(const QString &);
    void tiersReceived(const QStringList &tiers);
    void playerLogout(int);
    void sendRegister();
    void setReconnectPass(const QByteArray&);
    void cleanData();
    void onReconnectFailure(int reason);
    /* removes the pointer to the challenge window when it is destroyed */
    void clearChallenge();
    /* Display the info for that player */
    void seeInfo(int id, QString tier="");
    void seeInfo(QTreeWidgetItem *it);
    /* Challenge info by the server */
    void challengeStuff(const ChallengeInfo &c);
    /* Channels list */
    void channelCommandReceived(int command, int channel, DataStream *stream);
    void channelsListReceived(const QHash<qint32, QString> &channels);
    void sortChannels();
    void sortChannelsToggle(bool enabled);
    void channelPlayers(int chanid, const QVector<qint32> &ids = QVector<qint32>());
    void addChannel(const QString &name, int id);
    void channelNameChanged(int id, const QString &name);
    void removeChannel(int id);
    void leaveChannelR(int index);
    void leaveChannel(int id);
    void activateChannel(const QString& text);
    void join(const QString& text);
    void itemJoin(QListWidgetItem *);
    void lineJoin();
    void firstChannelChanged(int tabindex);
    void channelActivated(Channel *c);
    void pingActivated(Channel *c);
    void showChannelsContextMenu(const QPoint & point);
    /* battle... */
    void battleStarted(int battleid, const Battle &battle, const TeamBattle &team, const BattleConfiguration &conf);
    void battleStarted(int battleid, const Battle &battle);
    void battleReceived(int battleid, const Battle &battle);
    void battleFinished(int battleid, int res, int winner, int loser);
    void battleCommand(int battleid, const QByteArray &command);
    void saveBattleLogs(bool save);
    void animateHpBar(bool animate);
    void changeBattleLogFolder();
    void openSoundConfig();
    void forfeitBattle(int);
    void watchBattleOf(int);
    void watchBattleRequ(int);
    void watchBattle(int battleId, const BattleConfiguration &conf);
    void spectatingBattleMessage(int battleId, const QByteArray &command);
    void stopWatching(int battleId);
    void battleListActivated(QTreeWidgetItem* it);
    void loadTeam();
    /* A popup that asks for the pass */
    void askForPass(const QByteArray &salt);
    /* A popup that asks for a server pass */
    void serverPass(const QByteArray &salt);
    /* When someone is kicked, banned or temp banned */
    void playerKicked(int,int);
    void playerBanned(int,int);
    void playerTempBanned(int dest, int src, int time);
    /* When you kick someone */
    void kick(int);
    void ban(int);
    void tempban(int, int);
    void tempban60(int p) {
         tempban(p, 60);
    }
    void tempban1440(int p) {
         tempban(p, 1440);
    }

    void pmcp(QString);
    /* PM */
    void startPM(int);
    void closePM(int);
    void removePM(int id, const QString);
    void sendPM(int id, const QString);
    void PMReceived(int, const QString);
    /* CP */
    void controlPanel(int);
    void setPlayer(const UserInfo &ui);
    void requestBan(const QString &name);
    void requestTempBan(const QString &name, int time);
    /* Ranking */
    void seeRanking(int);
    /* Away... */
    void awayChanged(int id, bool away);
    void ladderChanged(int id, bool away);
    void goAway(int away);
    void goAwayB(bool away) {
        goAway(away);
    }
    void changeButtonStyle(bool old);
    void changeBattleWindow(bool old);
    void changeBattleLogger(bool logger);
    void changeBattleScreenSize(bool big);
    void changeBattleWeather(bool everyTurn);
    void changeNicknames(bool old);
    void enableLadder(bool);
    void sortPlayersByTiers(bool);
    void sortPlayersByAuth(bool);
    void setChannelSelected(int);
    void enablePlayerEvents();
    void disablePlayerEvents();
    void deleteCustomEvents();
    int getEventsForChannel(QString const& channel);
    void showPlayerEvents(bool b, int event, QString option);
    void showIdleEvents(bool);
    void showBattleEvents(bool);
    void showChannelEvents(bool);
    void showTeamEvents(bool);
    void toggleAutoJoin(bool autojoin);
    void toggleDefaultChannel(bool def);
    void toggleGlobalMessage(bool gmessage);
    bool ignoringGlobalMessage(const QString &channelName);
    void showTimeStamps(bool);
    void showTimeStamps2(bool);
    void showSeconds(bool);
    void toggleIncomingPM(bool);
    void togglePMTabs(bool);
    void togglePMNotifications(bool);
    void togglePMLogs(bool);
    void movePlayerList(bool);
    void useOldShortcuts(bool);
    void ignoreServerVersion(bool);
    void versionDiff(const ProtocolVersion &v, int level);
    void serverNameReceived(const QString &sName);
    void tierListReceived(const QByteArray &array);
    void changeTier();
    void openBattleFinder();
    void findBattle(const FindBattleData&);
    /* Ignored */
    void removeIgnore(int);
    void ignore(int);
    void ignore(int, bool);
    /* Teambuilder slots */
    void openTeamBuilder();
    void reloadTeamBuilderBar();
    void changeTeam();
    void changeTeam(const TeamHolder &t);
    /* Automatic removal of players in memory */
    void fadeAway();
    void registerPermPlayer(int id);
    QStringList const& eventSettings() const;
    void newConnection();
    /* exit warning */
    void changeExitWarning(bool show);
    void showExitWarning();
signals:
    void done();
    void userInfoReceived(const UserInfo &ui);
    void tierListFormed(const QStringList &tiers);
    void PMDisabled(bool value, int starterAuth);
    void togglePMs(bool value);
    void PMDisconnected(bool disconnected);
    void pmNotificationsChanged(bool notify);
    void titleChanged();
protected:
    void paintEvent(QPaintEvent *)
    {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

private:
    TeamHolder *myteam;
    MainEngine *top;
    QString mynick;

    /* GUI */
    /* Main chat */
    QScrollDownTextBrowser *mychat;
    /* PMs and disabled PMs */
    QHash<int, PMStruct*> mypms;
    QHash<QString, PMStruct*> disabledpms;
    /* Line the user types in */
//    QLineEdit *myline;
    QIRCLineEdit *myline;
    SmallPokeTextEdit *server_announcement;
    /* Where players are displayed */
    QStackedWidget *playersW, *battlesW;
    QExposedTabWidget *mainChat;
    QListWidget *channels;
    QLineEdit *channelJoin;
    /* Button to exit */
    QPushButton *myexit;
    /* Button to send text */
    QPushButton *mysender;
    /* Button to register a password */
    QPushButton *myregister;
    /* Button to find a battle */
    QPushButton *findMatch;
    /* PM System */
    PMSystem *pmSystem;

    /*Channels */
    QHash<qint32, QString> m_channelNames;
    QHash<QString, qint32> m_channelByNames;
    QHash<qint32, Channel *> mychannels;
    QStringList channelsIWasOn;
    /* Ignore */
    QList<int> myIgnored;

    /* Challenge ids, needed for accepting challenges with client scripts */
    QHash<int, ChallengeDialog *> mychallengeids;
    /* Challenge windows , to emit or to receive*/
    QSet<ChallengeDialog *> mychallenges;
    QPointer<FindBattleDialog> myBattleFinder;
    QHash<int, BaseBattleWindowInterface* > mySpectatingBattles;
    QHash<int, BattleWindow* > mybattles;
    QPointer<QAction> goaway;
    QPointer<QAction> ladder;

    bool findingBattle;
    bool isConnected, loggedIn;
    QString url;
    quint16 port;
    int _mid;
    int selectedChannel;
    QByteArray reconnectPass;

    ProtocolVersion serverVersion;
    QString serverName;

    QPointer<QMenuBar> mymenubar;
    QPointer<QMenu> mytiermenu;
    QList<QAction*> mytiers;
    QList<QAction*> myevents;
    QList<QAction*> mychanevents;
    /* You can call the teambuilder from here too */
    QPointer<TeamBuilder> myteambuilder;
    QStringList eventlist;

    QPointer<ControlPanel> myCP;
    QPointer<RankingDialog> myRanking;

    QHash<int, PlayerInfo> myplayersinfo;
    /* Players scheduled for deletion are put here */
    QHash<int, int> fade;
    /* Players which we have PMed are supposed to be kept in memory until they
       log out for real */
    QSet<int> pmedPlayers;
    QHash<QString, int> mynames;
    QHash<QString, int> mylowernames;

    QHash<qint32, Battle> battles;

    /* Network Relay */
    Analyzer *myrelay;
    Analyzer &relay();
    bool failedBefore;
public:
    Q_INVOKABLE Analyzer *network() {return myrelay;}
private:

    /* Password Wallet */
    PasswordWallet wallet;

    PlayerInfo & playerInfo(int id);
    void updateState(int player);
    /* Returns the challenge window displaying that player or NULL otherwise */
    ChallengeDialog * getChallengeWindow(int player);
    void closeChallengeWindow(ChallengeDialog *c);

    int freeChallengeId();

    void initRelay();
    void changeTiersChecked();
    void rebuildTierMenu();

    bool eventEnabled(int event);
    void ownPlayerReceived();

    TeamHolder secondTeam;
    bool waitingOnSecond;

    /* The mode of the tier list. If it's single, then a simple checkbox, otherwise another menu for each team for each tier */
    bool singleTeam;

    QSettings globals;
    bool exitWarning;

    QSet<OnlineClientPlugin*> plugins;
    PluginManager *pluginManager;
    QHash<OnlineClientPlugin*, QHash<QString, OnlineClientPlugin::Hook> > hooks;

    template<class T1>
    bool call(const QString &f, T1 arg1)
    {
        bool ret = true;
        foreach(OnlineClientPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (OnlineClientPlugin::*)(T1)>(hooks[p][f])))(arg1);
            }
        }

        return ret;
    }

    template<class T1, class T2>
    bool call(const QString &f, T1 arg1, T2 arg2)
    {
        bool ret = true;
        foreach(OnlineClientPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (OnlineClientPlugin::*)(T1, T2)>(hooks[p][f])))(arg1, arg2);
            }
        }

        return ret;
    }

    template<class T1, class T2, class T3>
    bool call(const QString &f, T1 arg1, T2 arg2, T3 arg3)
    {
        bool ret = true;
        foreach(OnlineClientPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (OnlineClientPlugin::*)(T1, T2, T3)>(hooks[p][f])))(arg1, arg2, arg3);
            }
        }

        return ret;
    }

    template<class T1, class T2, class T3, class T4>
    bool call(const QString &f, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
    {
        bool ret = true;
        foreach(OnlineClientPlugin *p, plugins) {
            if (hooks[p].contains(f)) {
                ret &= (*p.*(reinterpret_cast<int (OnlineClientPlugin::*)(T1, T2, T3, T4)>(hooks[p][f])))(arg1, arg2, arg3, arg4);
            }
        }

        return ret;
    }
};

#endif // CLIENT_H
