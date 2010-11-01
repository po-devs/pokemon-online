#include "loadinsertthread.h"
#include "sql.h"
#include "waitingobject.h"

void LoadThread::run()
{
    QString dbname = QString::number(long(QThread::currentThreadId()));

    SQLCreator::createSQLConnection(dbname);
    QSqlDatabase db = QSqlDatabase::database(dbname);
    QSqlQuery sql(db);
    sql.setForwardOnly(true);

    sem.acquire(1);

    forever {
        queryMutex.lock();
        Query q = queries.takeFirst();
        queryMutex.unlock();

        emit processQuery(&sql, q.data, q.query_type, q.w);
        q.w->emitSignal();

        sem.acquire(1);
    }
}


void LoadThread::pushQuery(const QVariant &name, WaitingObject *w, int query_type)
{
    queryMutex.lock();

    queries.push_back(Query(name, w, query_type));

    queryMutex.unlock();

    sem.release(1);
}
