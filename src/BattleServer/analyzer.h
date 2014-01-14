#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>

#include <Utilities/coreclasses.h>
#include <Utilities/asiosocket.h>
#include <PokemonInfo/networkstructs.h>

class GenericNetwork;
class ChallengeInfo;
class TeamBattle;
class BattlePlayer;
class BattleChoice;

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(GenericSocket sock, int id);
    
    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly, version.version);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emitCommand(tosend);
    }

    /* Convenience functions to avoid writing a new one every time */
    inline void emitCommand(const QByteArray &command) {
        emit sendCommand(command);
    }

    void dealWithCommand(const QByteArray & command);
signals:
    /* Network */
    void sendCommand(const QByteArray&);

    /* Server connection */
    void newBattle(int, const BattlePlayer&, const BattlePlayer&, const ChallengeInfo&, const TeamBattle&, const TeamBattle&);
    void playerChoice(int battle, int player, const BattleChoice&);
    void spectating(int battle, bool spectate, int player, const QString &name);
    void battleChat(int battle, int player, const QString &chat);
    void spectatingChat(int battle, int player, const QString &chat);
    void battleFinished(int battle, int result, int forfeiter);

    void modChanged(const QString &mod);
    void loadPlugin(const QString &path);
    void unloadPlugin(const QString &name);

    /* Network errors */
    void connectionError(int, const QString&);
    void disconnected();
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);
private:
    GenericNetwork *socket;
    int m_id;

    ProtocolVersion version;
};

#endif // ANALYZER_H
