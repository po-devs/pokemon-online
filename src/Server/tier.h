#ifndef TIER_H
#define TIER_H

#include <QtCore>
#include <QtGui>
#include "sql.h"
#include "memoryholder.h"

class TierMachine;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;

struct MemberRating
{
    QString name;
    int matches;
    int rating;

    MemberRating(const QString &name="") {
        rating = 1000;
        matches = 0;
        this->name = name;
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

    Tier(TierMachine *boss = NULL) : boss(boss) {}

    void fromString(const QString &s);
    QString toString() const;

    int rating(const QString &name) const {
        return holder.member(name).rating;
    }

    void changeRating(const QString &winner, const QString &loser);
    void changeRating(const QString &player, int newRating);
    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe);

    bool isBanned(const PokeBattle &p) const;
    bool isValid(const TeamBattle &t) const;
    bool exists(const QString &name) const;
    int ranking(const QString &name) const;
    void updateMember(const MemberRating &m);
    void loadMemberInMemory(const QString &name);

private:
    void loadFromFile();

    QString m_name;
    /* Used for table name in SQL database */
    QString sql_table;

    MemoryHolder<MemberRating> holder;

    WaitingObject* getObject();
    void freeObject(WaitingObject *c);
    MemberRating member(const QString &name);

    QSet<WaitingObject*> freeObjects;
    QSet<WaitingObject*> usedObjects;
};



#endif // TIER_H

