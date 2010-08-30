#ifndef LOADINSERTTHREAD_H
#define LOADINSERTTHREAD_H

#include <QtCore>
#include "sql.h"

class WaitingObject;
class QSqlQuery;

class LoadThread : public QThread
{
    Q_OBJECT
public:
    void pushQuery(const QVariant &name, WaitingObject *w, int query_type);

    void run();
signals:
    void processQuery (QSqlQuery *q, const QVariant &data, int query_type, WaitingObject *w);
private:
    struct Query {
        QVariant data;
        WaitingObject *w;
        int query_type;

        Query(const QVariant &m, WaitingObject *w, int query_type)
            : data(m), w(w), query_type(query_type)
        {

        }
    };

    QLinkedList<Query> queries;
    QMutex queryMutex;
    QSemaphore sem;
};

/* Qt doesn't manage templates and signals well, hence why the abstract class
   without templates */
class AbstractInsertThread : public QThread
{
    Q_OBJECT
public:
    virtual void run() = 0;

signals:
    void processMember (QSqlQuery *q, void * m, int type=1);
};

template <class T>
class InsertThread : public AbstractInsertThread
{
public:
    /* update/insert ? */
    void pushMember(const T &m, int desc);

    void run();
private:
    QLinkedList<QPair<T, int> > members;
    QMutex memberMutex;
    QSemaphore sem;
};


template <class T>
void InsertThread<T>::run()
{
    QString dbname = QString::number(intptr_t(QThread::currentThreadId()));

    SQLCreator::createSQLConnection(dbname);
    QSqlDatabase db = QSqlDatabase::database(dbname);
    QSqlQuery sql(db);
    sql.setForwardOnly(true);

    sem.acquire(1);

    forever {
        memberMutex.lock();
        QPair<T , int> p = members.takeFirst();
        memberMutex.unlock();

        emit processMember(&sql, &p.first, p.second);

        sem.acquire(1);
    }
}

template <class T>
void InsertThread<T>::pushMember(const T &member, int desc)
{
    memberMutex.lock();

    members.push_back(QPair<T, int> (member, desc) );

    memberMutex.unlock();

    sem.release(1);
}

#endif // LOADINSERTTHREAD_H
