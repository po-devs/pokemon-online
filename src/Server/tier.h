#ifndef TIER_H
#define TIER_H

#include <QtCore>
#include <QtGui>
#include "../Utilities/functions.h"
#include "../Utilities/rankingtree.h"

class TierMachine;
struct TeamBattle;
struct PokeBattle;

struct MemberRating
{
    PROPERTY(QString, name);
    PROPERTY(int, matches);
    PROPERTY(int, rating);
    PROPERTY(int, filePos);
    PROPERTY(RankingTree<QString>::iterator, node)

public:
    MemberRating() {
        rating() = 1000;
        matches() = 0;
    }

    QString toString() const;
    void changeRating(int other, bool win);
};

struct BannedPoke {
    int poke;
    int item;

    BannedPoke(int poke=0, int item=0):poke(poke),item(item) {}
};

inline uint qHash(const BannedPoke &p) {
    return qHash(p.poke + (p.item << 16));
}

struct Tier
{
    QString name;
    QString parent;
    TierMachine *boss;
    QMultiHash<int, BannedPoke> bannedPokes2; // The set is there to keep good perfs
    QList<BannedPoke> bannedPokes; // The list is there to keep the same order
    QFile *in;
    int lastFilePos;

    QHash<QString, MemberRating> ratings;
    RankingTree<QString> rankings;

    Tier(TierMachine *boss = NULL) : boss(boss), in(NULL) {}
    ~Tier() {
        delete in;
    }

    void loadFromFile();
    void fromString(const QString &s);
    QString toString() const;

    int rating(const QString &name) const {
        return ratings.value(name.toLower()).rating();
    }

    void changeRating(const QString &winner, const QString &loser);

    bool isBanned(const PokeBattle &p) const;
    bool isValid(const TeamBattle &t) const;
    bool exists(const QString &name) const {
        return ratings.contains(name.toLower());
    }
    int ranking(const QString &name) const {
        if (!exists(name))
            return -1;
        return ratings.value(name.toLower()).node()->ranking();
    }
};

class TierMachine
{
    PROPERTY(QString, tierList);
public:
    static void init();
    static TierMachine *obj();

    void save();

    QString toString()const;
    void fromString(const QString &s);

    const QList<QString>& tierNames() const;
    Tier& tier(const QString &name);
    const Tier& tier(const QString &name) const;
    bool exists(const QString &name) {
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

    const RankingTree<QString> * getRankingTree(const QString &tier);

    QString findTier(const TeamBattle &t) const;
private:
    QList<Tier> m_tiers;
    QList<QString> m_tierNames;
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

#endif // TIER_H

