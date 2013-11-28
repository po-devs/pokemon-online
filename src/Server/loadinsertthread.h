#ifndef LOADINSERTTHREAD_H
#define LOADINSERTTHREAD_H

#include <QtCore>
#include "waitingobject.h"

/* Qt doesn't manage templates and signals well, hence why the abstract class
   without templates */
class AbstractLoadInsertThread : public QThread
{
    Q_OBJECT
public:
    virtual void run() = 0;

signals:
    void processWrite (void * m, int type=1);
    void processLoad (const QVariant &data, int query_type, WaitingObject *w);
    void processDailyRun();
};

template <class T>
class LoadInsertThread : public AbstractLoadInsertThread
{
public:
    LoadInsertThread() : finished(false), dailyRunToProcess(false) { connect(this, SIGNAL(finished()), SLOT(deleteLater()));}
    ~LoadInsertThread() { finish(); }

    void pushQuery(const QVariant &name, WaitingObject *w, int query_type);
    void finish();

    void run();

    void pushMember(const T &m, int desc);
    void addDailyRun();

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
    bool finished;

    QLinkedList<QPair<T, int> > members;
    QMutex memberMutex;
    bool dailyRunToProcess;
};

template <class T>
void LoadInsertThread<T>::run()
{
    sem.acquire(1);

    forever {
        if (finished) {
            return;
        }

        bool loadQuery;

        {
            QMutexLocker l1(&queryMutex);
            QMutexLocker l2(&memberMutex);

            /* Maybe later a system to at least place a write now and then if overloaded with load queries */
            loadQuery = queries.size() > 0;
        }

        if (loadQuery) {
            queryMutex.lock();
            Query q = queries.takeFirst();
            queryMutex.unlock();

            emit processLoad(q.data, q.query_type, q.w);
            q.w->emitSignal();
        } else {
            if (dailyRunToProcess) {
                dailyRunToProcess = false;
                emit processDailyRun();
            } else {
                memberMutex.lock();
                QPair<T , int> p = members.takeFirst();
                memberMutex.unlock();

                emit processWrite(&p.first, p.second);
            }
        }

        sem.acquire(1);
    }
}

template <class T>
void LoadInsertThread<T>::pushMember(const T &member, int desc)
{
    memberMutex.lock();

    members.push_back(QPair<T, int> (member, desc) );

    memberMutex.unlock();

    sem.release(1);
}


template <class T>
void LoadInsertThread<T>::addDailyRun()
{
    dailyRunToProcess = true;

    sem.release(1);
}

template<class T>
void LoadInsertThread<T>::pushQuery(const QVariant &name, WaitingObject *w, int query_type)
{
    queryMutex.lock();

    queries.push_back(Query(name, w, query_type));

    queryMutex.unlock();

    sem.release(1);
}

template <class T>
void LoadInsertThread<T>::finish()
{
    finished = true;
    sem.release(1);
}

#endif // LOADINSERTTHREAD_H
