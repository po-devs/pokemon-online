#ifndef TIERMACHINE_H
#define TIERMACHINE_H

#include <QtGui>
#include <QtSql>
#include "../Utilities/functions.h"

class Tier;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;
class MemberRating;


class TierMachine
{
    PROPERTY(QString, tierList);
public:
    enum QueryType {
        GetInfoOnUser
    };

    static void init();
    static TierMachine *obj();

    void clear();
    void save();

    QString toString()const;
    void fromString(const QString &s);

    const QStringList& tierNames() const;
    Tier& tier(const QString &name);
    const Tier& tier(const QString &name) const;
    bool exists(const QString &name) const;
    bool existsPlayer(const QString &name, const QString &player);
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
    QList<Tier*> m_tiers;
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

#endif // TIERMACHINE_H
