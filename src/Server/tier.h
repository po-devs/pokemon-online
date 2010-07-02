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
class LoadThread;

struct MemberRating
{
    QString name;
    int matches;
    int rating;

    MemberRating(const QString &name="", int matches=0, int rating=1000) : name(name), matches(matches), rating(rating) {
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
    void changeId(int id);

    QString parent;
    TierMachine *boss;
    QMultiHash<int, BannedPoke> bannedPokes2; // The set is there to keep good perfs
    QList<BannedPoke> bannedPokes; // The list is there to keep the same order

    Tier(TierMachine *boss = NULL);

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
    void loadMemberInMemory(const QString &name, QObject *o=NULL, const char *slot=NULL);
    void processQuery(QSqlQuery *q, const QString &name, int type);
    void insertMember(QSqlQuery *q, void *data, int type);
protected:
    enum QueryType {
        GetInfoOnUser
    };

    int make_query_number(QueryType type);
    int id() const {
        return m_id;
    }

private:
    void loadFromFile();

    QString m_name;
    /* Used for table name in SQL database */
    QString sql_table;
    int m_id;

    MemoryHolder<MemberRating> holder;

    MemberRating member(const QString &name);

    LoadThread *getThread();
};



#endif // TIER_H

