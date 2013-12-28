#ifndef TIERMACHINE_H
#define TIERMACHINE_H

#include <QtGui>
#include "../Utilities/functions.h"
#include "tiertree.h"

class Tier;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;
class MemberRating;
class QSqlQuery;

template<class T> class LoadInsertThread;

class TierMachine : public QObject
{
    Q_OBJECT

    friend class Tier;
public:
    enum QueryType {
        GetInfoOnUser
    };

    static void init();
    static void destroy();
    static TierMachine *obj();

    TierMachine();
    ~TierMachine();

    void load();
    void loadDecaySettings();
    void clear();
    void save();

    QString toString()const;
    void fromString(const QString &s);

    const QStringList& tierNames() const;
    QByteArray tierList() const;
    Tier& tier(const QString &name);
    const Tier& tier(const QString &name) const;
    bool exists(const QString &name) const;
    bool existsPlayer(const QString &name, const QString &player);
    bool isValid(const TeamBattle &t, const QString tier) const;
    bool isBanned(const PokeBattle &p, const QString &tier) const;

    void loadMemberInMemory(const QString &name, const QString &tier, QObject *o, const char *slot);
    void fetchRankings(const QString &tier, const QVariant &data, QObject *o, const char *slot);

    int rating(const QString &name, const QString &tier);
    int inner_rating(const QString &name, const QString &tier);
    int ranking(const QString &name, const QString &tier);
    int count (const QString &tier);
    void changeRating(const QString &winner, const QString &loser, const QString &tier);
    void changeRating(const QString &winner, const QString &tier, int newRating);

    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe, const QString &tier);
    QString findTier(const TeamBattle &t) const;

    void exportDatabase() const;
    TierTree *getDataTree() const;

    static const int playersByPage = 40;

    int alt_expiration;
    int hours_per_period;
    int percent_per_period;
    int max_saved_periods;
    int max_percent_decay;
signals:
    void tiersChanged();
public slots:
    void processQuery(QSqlQuery *q, const QVariant &,int,WaitingObject*);
    void insertMember(QSqlQuery *q,void *,int);
    /* Processes the daily run in which ratings are updated.
       Be aware that it may take long. I may thread it in the future. */
    void processDailyRun();
private:
    QList<Tier*> m_tiers;
    QHash<QString, Tier*> m_tierByNames;
    QStringList m_tierNames;
    static TierMachine *inst;
    TierTree tree;
    QByteArray m_tierList;

    LoadInsertThread<MemberRating> *thread;

    static const int semaphoreMaxLoad = 1000;
    QSemaphore semaphore;

    LoadInsertThread<MemberRating> * getThread();

    /* Number gets increased by one every time tiers are reloaded.

        So that if tiers are reloaded while a threaded query was already thrown,
        if the version stored in the query and this version are different,
        the query is discarded. */
    volatile quint16 version;
};

/* For rankings */
typedef QVector<QPair<QString, int> > qvectorqpairqstringint;
Q_DECLARE_METATYPE(qvectorqpairqstringint)

#endif // TIERMACHINE_H
