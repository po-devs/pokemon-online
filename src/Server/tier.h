#ifndef TIER_H
#define TIER_H

#include <QtCore>
#include <QtGui>
#include "sql.h"
#include "../Utilities/functions.h"

class TierMachine;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;

struct MemberRating
{
    PROPERTY(QString, name);
    PROPERTY(int, matches);
    PROPERTY(int, rating);

public:
    MemberRating() {
        rating() = 1000;
        matches() = 0;
    }

    QString toString() const;
    void changeRating(int other, bool win);
    QPair<int, int> pointChangeEstimate(int otherRating);
};

struct BannedPoke {
    int poke;
    int item;

    BannedPoke(int poke=0, int item=0):poke(poke),item(item) {}
};

inline uint qHash(const BannedPoke &p) {
    return qHash(p.poke + (p.item << 16));
}

class Tier
{
    friend class TierMachine;
public:
    void changeName(const QString &name);
    QString name() const;

    QString parent;
    TierMachine *boss;
    QMultiHash<int, BannedPoke> bannedPokes2; // The set is there to keep good perfs
    QList<BannedPoke> bannedPokes; // The list is there to keep the same order

    Tier(TierMachine *boss = NULL) : boss(boss), memberMutex(new QMutex),
        cachedMembersMutex(new QMutex){}

    void fromString(const QString &s);
    QString toString() const;

    int rating(const QString &name) const {
        return members.value(name.toLower()).rating();
    }

    void changeRating(const QString &winner, const QString &loser);
    void changeRating(const QString &player, int newRating);
    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe);

    bool isBanned(const PokeBattle &p) const;
    bool isValid(const TeamBattle &t) const;
    bool exists(const QString &name) const {
        return members.contains(name.toLower());
    }
    int ranking(const QString &name) const {
        if (!exists(name))
            return -1;
        //return members.value(name.toLower()).node()->ranking();
        return 1;
    }

private:
    void loadFromFile();

    QString m_name;
    /* Used for table name in SQL database */
    QString sql_table;

    QHash<QString, MemberRating> members;
    QSet<QString> nonExistentMembers;
    QSharedPointer<QMutex> memberMutex;
    QLinkedList<QString> cachedMembersOrder;
    QSharedPointer<QMutex> cachedMembersMutex;
    QSet<QString> bannedIPs;
    QHash<QString, QString> bannedMembers;

    WaitingObject* getObject();
    void freeObject(WaitingObject *c);

    QSet<WaitingObject*> freeObjects;
    QSet<WaitingObject*> usedObjects;
};



#endif // TIER_H

