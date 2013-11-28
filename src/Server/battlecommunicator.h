#ifndef BATTLECOMMUNICATOR_H
#define BATTLECOMMUNICATOR_H

#define XSUFFIX(x) SUFFIX(x)
#define SUFFIX(x) #x
#define BATTLE_SERVER_SUFFIX XSUFFIX(EXE_SUFFIX)

#include <QObject>
#include <QString>
#include <QHash>
#include <QProcess>

#include "../Utilities/coreclasses.h"

class BattleAnalyzer;
class BattleChoice;
class Player;
class ChallengeInfo;
class FullBattleConfiguration;
class BattleConfiguration;
class TeamBattle;

class BattleCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit BattleCommunicator(QObject *parent = 0);
    
    /* Number of ongoing battles */
    int count() const;
    bool contains(int battleid) const;

    /* Can we have battles? */
    bool valid() const;

    void startBattle(Player *p1, Player *p2, const ChallengeInfo &c, int id, int team1, int team2);

    /* Sent from the server */
    void playerForfeit(int battleid, int forfeiter);
    void removeBattle(int battleid);

    void addSpectator(int battleid, int id, const QString &name);
    void removeSpectator(int battleid, int id);

    FullBattleConfiguration *battle(int battleid);
signals:
    void info(const QString &message);
    void error();
    void battleInfo(int,int,const QByteArray&);
    void battleFinished(int,int,int,int);
    void sendBattleInfos(int,int,int,const TeamBattle&,const BattleConfiguration&,const QString&);
public slots:
    void startServer();
    void connectToBattleServer();
    void battleConnected();
    void battleConnectionError();

    /* Player -> battle server */
    void battleMessage(int player, int battle, const BattleChoice &choice);
    void resendBattleInfos(int player, int battle);
    void battleChat(int player, int battle, const QString &chat);
    void spectatingChat(int player, int battle, const QString &chat);
    /* Battle server -> player */
    void filterBattleInfos(int,int,int,const TeamBattle&,const BattleConfiguration&,const QString&);
    void filterBattleInfo(int battle, int player, const QByteArray &info);
    void filterBattleResult(int, int, int, int);
    /* Server -> Battle server */
    void changeMod(const QString &mod);
private slots:
    void battleServerStarted();
    void battleServerError(QProcess::ProcessError error);
    void battleServerFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    BattleAnalyzer* relay;
    QProcess* battleServer;

    QHash<int, FullBattleConfiguration*> mybattles;
    QString mod;

    void showResult(int battle, int result, int loser);

    const QString processErrorMessages[6] = {
        "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.",
        "The process crashed some time after starting successfully.",
        "The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again.",
        "An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.",
        "An error occurred when attempting to read from the process. For example, the process may not be running.",
        "An unknown error occurred. This is the default return value of error()."
    };

    const QString processExitMessages[2] = {
        "closed"
        "crashed"
    };

    template <typename ...Params>
    QByteArray pack(int command, int who, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), qint8(who), std::forward<Params>(params)...);

        return tosend;
    }

};

#endif // BATTLECOMMUNICATOR_H
