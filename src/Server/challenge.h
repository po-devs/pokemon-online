#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <QObject>
#include <PokemonInfo/battlestructs.h>

class Server;
class Player;

/* Warning: some of the public methods delete the object (but they remove the references
   of the class from the players beforehand.

   That means you should only use 1 function at a time */
class Challenge : public QObject
{
    Q_OBJECT
public:
    Challenge (Player *source, Player *dest, const ChallengeInfo &c, Server *s);

    void manageStuff(Player *source, const ChallengeInfo &c);
    void cancel(Player *p, bool refused = false);
    void cancelFromServer();
    void onPlayerDisconnect(Player *p);

    int challenger() const;
    QString tier() const;
    Pokemon::gen gen() const;
    int challenged() const;
    ChallengeInfo description() const { return desc; }

    class Exception {

    };

signals:
    void battleStarted(int src, int dest, const ChallengeInfo &desc, int srcteam, int destteam);
private:
    Player* src;
    Player* dest;
    ChallengeInfo desc;
    Server *server;
    bool cancelledFromServer;
};


#endif // CHALLENGE_H
