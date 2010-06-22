#ifndef TIER_H
#define TIER_H

#include <QtCore>
#include <QtGui>
#include "sql.h"
#include "waitingobject.h"
#include "../Utilities/functions.h"

class TierMachine;
struct TeamBattle;
struct PokeBattle;

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

class TierMachine
{
    PROPERTY(QString, tierList);
public:
    enum QueryType {
        GetInfoOnUser
    };

    static void init();
    static TierMachine *obj();

    void save();

    QString toString()const;
    void fromString(const QString &s);

    const QStringList& tierNames() const;
    Tier& tier(const QString &name);
    const Tier& tier(const QString &name) const;
    bool exists(const QString &name) const {
        return m_tierNames.contains(name);
    }
    bool existsPlayer(const QString &name, const QString &player)
    {
        return exists(name) && tier(name).exists(player);
    }

    bool isValid(const TeamBattle &t, const QString tier) const;
    bool isBanned(const PokeBattle &p, const QString &tier) const;

    int rating(const QString &name, const QString &tier);
    int ranking(const QString &name, const QString &tier);
    int count (const QString &tier);
    void changeRating(const QString &winner, const QString &loser, const QString &tier);
    void changeRating(const QString &winner, const QString &tier, int newRating);

    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe, const QString &tier);

    QString findTier(const TeamBattle &t) const;
private:
    QList<Tier> m_tiers;
    QStringList m_tierNames;
    static TierMachine *inst;
};

class TierWindow : public QWidget
{
    Q_OBJECT
public:
    TierWindow(QWidget *parent = NULL);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QPlainTextEdit *m_editWindow;
};

class LoadTThread : public QThread
{
public:
    void pushQuery(const QString &name, WaitingObject *w, TierMachine::QueryType query_type);

    void run();

    static void processQuery (QSqlQuery *q, const QString &name, TierMachine::QueryType query_type);
private:
    struct Query {
        QString member;
        WaitingObject *w;
        TierMachine::QueryType query_type;

        Query(const QString &m, WaitingObject *w, TierMachine::QueryType query_type)
            : member(m), w(w), query_type(query_type)
        {

        }
    };

    QLinkedList<Query> queries;
    QMutex queryMutex;
    QSemaphore sem;
};

class InsertTThread : public QThread
{
public:
    /* update/insert ? */
    void pushMember(const MemberRating &m, bool update=true);

    void run();

    static void processMember (QSqlQuery *q, const MemberRating &m, bool update=true);
private:
    QLinkedList<QPair<MemberRating, bool> > members;
    QMutex memberMutex;
    QSemaphore sem;
};


#endif // TIER_H

