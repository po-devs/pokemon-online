#ifndef PLAYER_H
#define PLAYER_H

#include "../PokemonInfo/networkstructs.h"
#include "analyze.h"

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
    Player(QTcpSocket *sock);
    ~Player();

    /* returns all the regular info */
    TeamInfo &team();
    const TeamInfo &team() const;
    /* Converts the content of the TeamInfo to a basicInfo and returns it */
    BasicInfo basicInfo() const;

    /* Sends a message to the player */
    void sendMessage(const QString &mess);

    void setId(int id);
    int id() const;
    QString name() const;

    bool isLoggedIn() const;
    void setLoggedIn(bool logged);

    bool connected() const;

    bool isChallenged() const;
    bool hasChallenged() const;
    int challengedBy() const;

    /* Sends the challenge, returns false if can't even send the challenge */
    bool challenge(int id);

    /* Confirms the sending of a challenge */
    void challengeIssued(int id);
    void sendBusyForChallenge(int id);
    void startBattle(int id);
    void sendChallengeRefusal(int id);
    void sendChallengeCancel(int id);
    void cancelChallenges();

    Analyzer& relay();
    const Analyzer& relay() const;
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id);
    void challengeFromTo(int idfrom, int idto);
    void challengeAcc(int idfrom, int idto);
    void challengeRef(int idfrom, int idto);
    void challengeCanceled(int idfrom, int idto);
    void busyForChallenge(int idfrom, int idto);
public slots:
    void loggedIn(const TeamInfo &team);
    void recvMessage(const QString &mess);
    void recvTeam(const TeamInfo &team);
    void disconnected();
    void challengeReceived(int id);
    void challengeRefused(int id);
    void challengeAccepted(int id);
    void busyForChallenge(int id);
private:
    TeamInfo myteam;
    Analyzer myrelay;
    int myid;

    QSet<int> m_challenged;
    int m_challengedby;
    bool m_isChallenged;

    bool m_isLoggedIn;

    void removeChallenge(int id);
};

#endif // PLAYER_H
