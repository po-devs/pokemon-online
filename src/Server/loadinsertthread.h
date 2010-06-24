#ifndef LOADINSERTTHREAD_H
#define LOADINSERTTHREAD_H

#include <QtCore>
#include <QtSql>
#include "sql.h"

class WaitingObject;

class LoadThread : public QThread
{
    Q_OBJECT
public:
    void pushQuery(const QString &name, WaitingObject *w, int query_type);

    void run();
signals:
    void processQuery (QSqlQuery *q, const QString &name, int query_type);
private:
    struct Query {
        QString member;
        WaitingObject *w;
        int query_type;

        Query(const QString &m, WaitingObject *w, int query_type)
            : member(m), w(w), query_type(query_type)
        {

        }
    };

    QLinkedList<Query> queries;
    QMutex queryMutex;
    QSemaphore sem;
};

class AbstractInsertThread : public QThread
{
    Q_OBJECT
public:
    virtual void run() = 0;

signals:
    void processMember (QSqlQuery *q, void * m, bool update=true);
};

template <class T>
class InsertThread : public AbstractInsertThread
{
public:
    /* update/insert ? */
    void pushMember(const T &m, bool update=true);

    void run();
private:
    QLinkedList<QPair<T, bool> > members;
    QMutex memberMutex;
    QSemaphore sem;
};


template <class T>
void InsertThread<T>::run()
{
    QString dbname = QString::number(int(QThread::currentThreadId()));

    SQLCreator::createSQLConnection(dbname);
    QSqlDatabase db = QSqlDatabase::database(dbname);
    QSqlQuery sql(db);
    sql.setForwardOnly(true);

    sem.acquire(1);

    forever {
        memberMutex.lock();
        QPair<T , bool> p = members.takeFirst();
        memberMutex.unlock();

        emit processMember(&sql, &p.first, p.second);

        sem.acquire(1);
    }
}

template <class T>
void InsertThread<T>::pushMember(const T &member, bool update)
{
    memberMutex.lock();

    members.push_back(QPair<T, bool> (member, update) );

    memberMutex.unlock();

    sem.release(1);
}

#endif // LOADINSERTTHREAD_H
