#include "security.h"
#include "sql.h"
#include <QtSql>
#include "../Utilities/otherwidgets.h"
#include <ctime>
#include "server.h"
#include "waitingobject.h"

QHash<QString, SecurityManager::Member> SecurityManager::members;
QNickValidator SecurityManager::val(NULL);
QSet<QString> SecurityManager::bannedIPs;
QSet<QString> SecurityManager::bannedMembers;
QSet<WaitingObject*> SecurityManager::freeObjects;
QSet<WaitingObject*> SecurityManager::usedObjects;
QLinkedList<QString> SecurityManager::cachedMembersOrder;
QSet<QString> SecurityManager::nonExistentMembers;
QMutex SecurityManager::memberMutex;
QMutex SecurityManager::cachedMembersMutex;
int SecurityManager::nextLoadThreadNumber = 0;
LoadThread * SecurityManager::threads = NULL;
InsertThread * SecurityManager::ithread = NULL;

void SecurityManager::loadMembers()
{
    QSqlQuery query;

    query.exec("select * from trainers limit 1");

    if (!query.next()) {
        if (SQLCreator::databaseType == SQLCreator::PostGreSQL) {
            /* The only way to have an auto increment field with PostGreSQL is to my knowledge using the serial type */
            query.exec("create table trainers (id serial, "
                                                 "name varchar(20), laston char(10), auth int, banned boolean,"
                                                 "salt varchar(7), hash varchar(32), ip varchar(39), primary key(id), unique(name))");

        } else if (SQLCreator::databaseType == SQLCreator::SQLite){
            /* The only way to have an auto increment field with SQLite is to my knowledge having a 'integer primary key' field -- that exact quote */
            query.exec("create table if not exists trainers (id integer primary key, name varchar(20) unique, "
                            "laston char(10), auth int, banned boolean, salt varchar(7), hash varchar(32), "
                            "ip varchar(39)));");
        } else {
            throw QString("Using a not supported database");
        }

        query.exec("create index tname_index on trainers (name)");
        query.exec("create index tip_index on trainers (ip)");

        Server::print("importing old db");
        QFile memberFile("members.txt");

        if (!memberFile.open(QFile::ReadWrite)) {
            throw QObject::tr("Error: cannot open the file that contains the members ");
        }

        clock_t t = clock();

        query.prepare("insert into trainers(name, laston, auth,  banned, salt, hash, ip) values (:name, :laston, :auth,"
                      ":banned, :salt, :hash, :ip)");

        while (!memberFile.atEnd()) {
            QByteArray arr = memberFile.readLine();
            QString s = QString::fromUtf8(arr.constData(), std::max(0,arr.length()-1)); //-1 to remove the \n

            QStringList ls = s.split('%');

            if (ls.size() == 6 && isValid(ls[0])) {
                query.bindValue(":name", ls[0]);
                query.bindValue(":laston",ls[1]);
                query.bindValue(":auth", ls[2][0].toAscii()-'0');
                query.bindValue(":banned", ls[2][1] == '1');
                query.bindValue(":salt", ls[3].trimmed());
                query.bindValue(":hash", ls[4].trimmed());
                query.bindValue(":ip", ls[5].trimmed());
                query.exec();
            }
        }


        t = clock() - t;

        Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
        Server::print(query.lastError().text());
    }

    query.exec("select * from trainers limit 1");

    if (query.next())
        for (int i = 0; i < 8; i++)
            Server::print(query.value(i).toString());
}

SecurityManager::Member::Member(const QString &name, const QByteArray &date, int auth, bool banned, const QByteArray &salt, const QByteArray &hash,
                                const QByteArray &ip)
    :name(name.toLower()), date(date), auth(auth), banned(banned), salt(salt), hash(hash), ip(ip)
{
}

void SecurityManager::init()
{

    threads = new LoadThread[loadThreadCount];

    for (int i = 0; i < loadThreadCount; i++) {
        threads[i].start();
    }

    ithread = new InsertThread();
    ithread->start();


    loadMembers();
}

bool SecurityManager::isValid(const QString &name) {
    return val.validate(name) == QValidator::Acceptable;
}

bool SecurityManager::exist(const QString &name)
{
    QString n2 = name.toLower();

    {
        QMutexLocker m(&memberMutex);

        if (nonExistentMembers.contains(n2))
            return false;

        if (members.contains(n2)) {
            return true;
        }
    }

    loadMemberInMemory(n2);

    return exist(n2);
}

SecurityManager::Member SecurityManager::member(const QString &name)
{
    QString n2 = name.toLower();
    /* Exist will make a call to loadInMemory if needed */
    if (exist(n2)) {
        QMutexLocker lock(&memberMutex);
        return members[n2];
    } else {
        qDebug() << "Critical! Unreachable code reached";
        return member(name);
    }
}


QList<QString> SecurityManager::membersForIp(const QString &ip)
{
    return QList<QString>();
}

QSet<QString> SecurityManager::banList()
{
    return bannedMembers;
}

void SecurityManager::create(const QString &name, const QString &date, const QString &ip) {
    Member m(name.toLower(), date.toAscii(), 0, false, "", "", ip.toAscii());
    addMemberInMemory(m);
    updateMemberInDatabase(m, true);
}

void SecurityManager::addNonExistant(const QString &name)
{
    memberMutex.lock();

    nonExistentMembers.insert(name.toLower());

    memberMutex.unlock();
}

void SecurityManager::updateMemberInDatabase(const Member &m, bool add)
{
    ithread->pushMember(m, !add);
}

void SecurityManager::updateMember(const Member &m) {
    addMemberInMemory(m);

    updateMemberInDatabase(m, false);
}

void SecurityManager::addMemberInMemory(const Member &m)
{
    memberMutex.lock();

    nonExistentMembers.remove(m.name);
    if (!members.contains(m.name))
        cachedMembersOrder.push_front(m.name);

    members[m.name] = m;

    memberMutex.unlock();
}

void SecurityManager::cleanCache()
{
    cachedMembersMutex.lock();

    while(cachedMembersOrder.size() > 10000) {
        removeMemberInMemory(cachedMembersOrder.takeLast());
    }

    cachedMembersMutex.unlock();
}

void SecurityManager::removeMemberInMemory(const QString &name)
{
    memberMutex.lock();

    members.remove(name);

    memberMutex.unlock();
}

bool SecurityManager::bannedIP(const QString &ip) {
    return bannedIPs.contains(ip);
}

void SecurityManager::ban(const QString &name) {
    QString name2 = name.toLower();
    if (exist(name2)) {
        members[name2].ban();
        bannedMembers.insert(name2);
        bannedIPs.insert(members[name2].ip.trimmed());
        updateMember(members[name2]);
    }
}

void SecurityManager::unban(const QString &name) {
    QString name2 = name.toLower();
    if (exist(name2)) {
        IPunban(members[name2].ip.trimmed());
    }
}

void SecurityManager::IPunban(const QString &ip)
{
    QList<QString> _members = membersForIp(ip);

    foreach(QString name, _members)
    {
        members[name].unban();
        bannedMembers.remove(name);
        updateMember(members[name]);
    }
    bannedIPs.remove(ip);
}

void SecurityManager::setauth(const QString &name, int auth) {
    QString name2 = name.toLower();
    if (exist(name2)) {
        members[name2].setAuth(auth);
        updateMember(members[name2]);
    }
}

void SecurityManager::clearPass(const QString &name) {
    QString name2 = name.toLower();
    if (exist(name2)) {
        members[name2].clearPass();
        updateMember(members[name2]);
    }
}

int SecurityManager::maxAuth(const QString &ip) {
    int max = 0;
/*
    QStringList l = playersByIp.values(ip);

    foreach(QString name, l) {
        max = std::max(max, member(name).authority());
    }
*/
    return max;
}

QString SecurityManager::ip(const QString &name)
{
    QString name2 = name.toLower();
    if (exist(name2))
        return members[name2].ip.trimmed();
    else
        return "";
}

void SecurityManager::loadMemberInMemory(const QString &name, QObject *o, const char *slot)
{
    QString n2 = name.toLower();

    if (o == NULL) {
        if (isInMemory(n2))
            return;

        QSqlQuery q;
        LoadThread::processQuery(&q, name, GetInfoOnUser);

        return;
    }

    cleanCache();

    WaitingObject *w = getObject();

    connect(w, SIGNAL(waitFinished()), o, slot);

    if (isInMemory(n2))
        w->emitSignal();
    else {
        usedObjects.insert(w);

        LoadThread *t = getThread();

        t->pushQuery(n2, w, GetInfoOnUser);
    }
}

bool SecurityManager::isInMemory(const QString &name)
{
    QString n2 = name.toLower();

    QMutexLocker lock(&memberMutex);
    return members.contains(n2) || nonExistentMembers.contains(n2);
}

WaitingObject * SecurityManager::getObject()
{
    if (!freeObjects.isEmpty()) {
        WaitingObject *w = *freeObjects.begin();
        freeObjects.remove(w);
        return w;
    } else {
        return new WaitingObject();
    }
}

void SecurityManager::freeObject()
{
    usedObjects.remove((WaitingObject*)sender());
    freeObjects.insert((WaitingObject*)sender());
}

LoadThread * SecurityManager::getThread()
{
    int n = nextLoadThreadNumber;
    nextLoadThreadNumber = (nextLoadThreadNumber + 1) % loadThreadCount;
    return threads + n;
}

SecurityManager * SecurityManager::instance = new SecurityManager();


void LoadThread::run()
{
    QString dbname = QString::number(int(QThread::currentThreadId()));

    SQLCreator::createSQLConnection(dbname);
    QSqlDatabase db = QSqlDatabase::database(dbname);
    QSqlQuery sql(db);
    sql.setForwardOnly(true);

    sem.acquire(1);

    forever {
        queryMutex.lock();
        Query q = queries.takeFirst();
        queryMutex.unlock();

        processQuery(&sql, q.member, q.query_type);
        q.w->emitSignal();

        sem.acquire(1);
    }
}


void LoadThread::pushQuery(const QString &name, WaitingObject *w, SecurityManager::QueryType query_type)
{
    queryMutex.lock();

    queries.push_back(Query(name, w, query_type));

    queryMutex.unlock();

    sem.release(1);
}

void LoadThread::processQuery(QSqlQuery *q, const QString &name, SecurityManager::QueryType query_type)
{
    if (query_type == SecurityManager::GetInfoOnUser) {
        q->prepare("select laston, auth, banned, salt, hash, ip from trainers where name=? limit 1");
        q->addBindValue(name);
        q->exec();
        if (!q->next()) {
            SecurityManager::addNonExistant(name);
        } else {
            SecurityManager::Member m(name, q->value(0).toByteArray(), q->value(1).toInt(), q->value(2).toBool(), q->value(3).toByteArray(),
                                      q->value(4).toByteArray(), q->value(5).toByteArray());
            SecurityManager::addMemberInMemory(m);
        }
    }
}

void InsertThread::run()
{
    QString dbname = QString::number(int(QThread::currentThreadId()));

    SQLCreator::createSQLConnection(dbname);
    QSqlDatabase db = QSqlDatabase::database(dbname);
    QSqlQuery sql(db);
    sql.setForwardOnly(true);

    sem.acquire(1);

    forever {
        memberMutex.lock();
        QPair<SecurityManager::Member, bool> p = members.takeFirst();
        memberMutex.unlock();

        processMember(&sql, p.first, p.second);

        sem.acquire(1);
    }
}


void InsertThread::pushMember(const SecurityManager::Member &member, bool update)
{
    memberMutex.lock();

    members.push_back(QPair<SecurityManager::Member, bool> (member, update) );

    memberMutex.unlock();

    sem.release(1);
}

void InsertThread::processMember(QSqlQuery *q, const SecurityManager::Member &m, bool update)
{
    q->finish();
    if (update)
        q->prepare("update trainers set laston=:laston, auth=:auth, banned=:banned, salt=:salt, hash=:hash, ip=:ip where name=:name");
    else
        q->prepare("insert into trainers(name, laston, auth, banned, salt, hash, ip) values(:name, :laston, :auth, :banned, :salt, :hash, :ip)");

    q->bindValue(":name", m.name);
    q->bindValue(":laston", m.date);
    q->bindValue(":auth", m.auth);
    q->bindValue(":banned", m.banned);
    q->bindValue(":hash", m.hash);
    q->bindValue(":salt", m.salt);
    q->bindValue(":ip", m.ip);

    q->exec();
}
