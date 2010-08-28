#ifndef TIER_H
#define TIER_H

#include <QtCore>
#include <QtGui>
#include "sql.h"
#include "memoryholder.h"

class TierMachine;
class TierCategory;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;
class LoadThread;
class QDomElement;

struct MemberRating
{
    QString name;
    int matches;
    int rating;

    MemberRating(const QString &name="", int matches=0, int rating=1000) : name(name.toLower()), matches(matches), rating(rating) {
    }

    QString toString() const;
    void changeRating(int other, bool win);
    QPair<int, int> pointChangeEstimate(int otherRating);
};

struct BannedPoke {
    int poke;
    int item;
    QSet<int> moves;

    BannedPoke(int poke=0, int item=0):poke(poke),item(item) {}

    void loadFromXml(const QDomElement &elem);
};

inline uint qHash(const BannedPoke &p) {
    return qHash(p.poke + (p.item << 16));
}

class Tier
{
    friend class TierMachine;
    friend class ScriptEngine;
public:
    void changeName(const QString &name);
    QString name() const;
    void changeId(int newid);

    Tier(TierMachine *boss, TierCategory *cat);

    QString toString() const;

    int rating(const QString &name);

    void changeRating(const QString &winner, const QString &loser);
    void changeRating(const QString &player, int newRating);
    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe);

    void addBanParent(Tier *t);

    bool isBanned(const PokeBattle &p) const;
    bool isValid(const TeamBattle &t) const;
    bool exists(const QString &name);
    int ranking(const QString &name);
    int count();
    void updateMember(const MemberRating &m, bool add=false);
    void updateMemberInDatabase(const MemberRating &m, bool add=false);
    void loadMemberInMemory(const QString &name, QObject *o=NULL, const char *slot=NULL);
    void fetchRankings(const QVariant &data, QObject *o, const char *slot);
    void processQuery(QSqlQuery *q, const QVariant &name, int type, WaitingObject *w);
    void insertMember(QSqlQuery *q, void *data, int type);

    void exportDatabase() const;
    /* Load tier configuration */
    void loadFromXml(const QDomElement &elem);
    /* Load tier ladders */
    void loadFromFile();
    /* Kills and deletes itself, as well as from the category parent. Beware to not use any member functions after */
    void kill();
protected:
    enum QueryType {
        GetInfoOnUser,
        GetRankings,
    };

    int make_query_number(int type);
    int id() const {
        return m_id;
    }
private:
    TierMachine *boss;
    TierCategory *node;

    bool banPokes;
    QMultiHash<int, BannedPoke> bannedPokes2; // The set is there to keep good perfs
    QList<BannedPoke> bannedPokes; // The list is there to keep the same order
    QMultiHash<int, BannedPoke> restrictedPokes2;
    QList<BannedPoke> restrictedPokes;
    int maxRestrictedPokes;
    int numberOfPokemons;
    int maxLevel;
    int gen;
    QString banParentS;
    Tier *parent;
    QSet<int> bannedItems;
    QSet<int> bannedMoves;
    int doubles; /* < 0 : singles, 0: either, > 0: doubles */
    quint32 clauses;

    QString m_name;
    /* Used for table name in SQL database */
    QString sql_table;
    int m_id;
    int m_count;
    int last_count_time;

    mutable MemoryHolder<MemberRating> holder;

    MemberRating member(const QString &name);

    LoadThread *getThread();
};

#endif // TIER_H
