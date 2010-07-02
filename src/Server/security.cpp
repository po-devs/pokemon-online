#include "security.h"
#include "sql.h"
#include <QtSql>
#include "../Utilities/otherwidgets.h"
#include <ctime>
#include "server.h"
#include "waitingobject.h"

MemoryHolder<SecurityManager::Member>  SecurityManager::holder;
QNickValidator SecurityManager::val(NULL);
QSet<QString> SecurityManager::bannedIPs;
QHash<QString, QString> SecurityManager::bannedMembers;
int SecurityManager::nextLoadThreadNumber = 0;
LoadThread * SecurityManager::threads = NULL;
InsertThread<SecurityManager::Member> * SecurityManager::ithread = NULL;

void SecurityManager::loadMembers()
{
    QSqlQuery query;

    query.setForwardOnly(true);

    query.exec("select * from trainers limit 1");

    if (!query.next()) {
        if (SQLCreator::databaseType == SQLCreator::PostGreSQL) {
            /* The only way to have an auto increment field with PostGreSQL is to my knowledge using the serial type */
            query.exec("create table trainers (id serial, "
                                                 "name varchar(20), laston char(10), auth int, banned boolean,"
                                                 "salt varchar(7), hash varchar(32), ip varchar(39), primary key(id), unique(name))");
        } else if (SQLCreator::databaseType == SQLCreator::MySQL) {
            query.exec("CREATE TABLE IF NOT EXISTS trainers (id int(11) NOT NULL auto_increment, "
                                                            "name varchar(20), laston char(10), auth int(11), banned bool, "
                                                            "salt varchar(7), hash varchar(32), ip varchar(39), "
                                                            "PRIMARY KEY (id));");
        } else if (SQLCreator::databaseType == SQLCreator::SQLite){
            /* The only way to have an auto increment field with SQLite is to my knowledge having a 'integer primary key' field -- that exact quote */
            query.exec("create table trainers (id integer primary key autoincrement, name varchar(20) unique, "
                            "laston char(10), auth int, banned boolean, salt varchar(7), hash varchar(32), "
                            "ip varchar(39));");
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
                /* Weirdly, i seem to have problems when updating something that has a salt containing \, probably postgresql driver,
                   so i remove them. */
                if (!ls[3].contains('\\')) {
                    query.bindValue(":salt", ls[3].trimmed());
                    query.bindValue(":hash", ls[4].trimmed());
                } else {
                    query.bindValue(":salt", "");
                    query.bindValue(":hash", "");
                }
                query.bindValue(":ip", ls[5].trimmed());
                query.exec();
            }
        }


        t = clock() - t;

        Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
        Server::print(query.lastError().text());
    }

    /* Loading the ban list */
    query.exec("select name, ip from trainers where banned=true");

    while (query.next()) {
        bannedIPs.insert(query.value(1).toString());
        bannedMembers.insert(query.value(0).toString(), query.value(1).toString());
    }

//    //Uncomment if you want to test the database connection
//    query.exec("select * from trainers limit 1");
//
//    if (query.next())
//        for (int i = 0; i < 8; i++)
//            Server::print(query.value(i).toString());
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
        connect(&threads[i], SIGNAL(processQuery (QSqlQuery *, QString, int)), instance, SLOT(loadMember(QSqlQuery*,QString,int)), Qt::DirectConnection);
        threads[i].start();
    }

    ithread = new InsertThread<Member>();
    connect(ithread, SIGNAL(processMember(QSqlQuery*,void*,int)), instance, SLOT(insertMember(QSqlQuery*,void*,int)), Qt::DirectConnection);

    ithread->start();


    loadMembers();
}

bool SecurityManager::isValid(const QString &name) {
    return val.validate(name) == QValidator::Acceptable;
}

bool SecurityManager::exist(const QString &name)
{
    if (!holder.isInMemory(name)) {
        loadMemberInMemory(name);
    }

    return holder.exists(name);
}

SecurityManager::Member SecurityManager::member(const QString &name)
{
    if (!holder.isInMemory(name)) {
        loadMemberInMemory(name);
    }

    return holder.member(name);
}


QStringList SecurityManager::membersForIp(const QString &ip)
{
    QSqlQuery q;
    q.setForwardOnly(true);

    q.prepare("select name from trainers where ip=?");
    q.addBindValue(ip);
    q.exec();

    QStringList ret;

    while (q.next()) {
        ret.push_back(q.value(0).toString());
    }

    return ret;
}

QHash<QString, QString> SecurityManager::banList()
{
    return bannedMembers;
}

void SecurityManager::create(const QString &name, const QString &date, const QString &ip) {
    Member m(name.toLower(), date.toAscii(), 0, false, "", "", ip.toAscii());
    holder.addMemberInMemory(m);
    updateMemberInDatabase(m, true);
}

void SecurityManager::updateMemberInDatabase(const Member &m, bool add)
{
    ithread->pushMember(m, !add);
}

void SecurityManager::updateMember(const Member &m) {
    holder.addMemberInMemory(m);

    updateMemberInDatabase(m, false);
}

bool SecurityManager::bannedIP(const QString &ip) {
    return bannedIPs.contains(ip);
}

void SecurityManager::ban(const QString &name) {
    if (exist(name)) {
        Member m = member(name);
        m.ban();

        bannedMembers.insert(name.toLower(), m.ip);
        bannedIPs.insert(m.ip);

        updateMember(m);
    }
}

void SecurityManager::unban(const QString &name) {
    if (exist(name)) {
        Member m = member(name);
        IPunban(m.ip);
    }
}

void SecurityManager::IPunban(const QString &ip)
{
    QList<QString> _members = membersForIp(ip);

    foreach(QString name, _members)
    {
        Member m = member(name);
        m.unban();
        bannedMembers.remove(name);
        updateMember(m);
    }
    bannedIPs.remove(ip);
}

void SecurityManager::setauth(const QString &name, int auth) {
    if (exist(name)) {
        Member m = member(name);
        m.setAuth(auth);
        updateMember(m);
    }
}

void SecurityManager::clearPass(const QString &name) {
    if (exist(name)) {
        Member m = member(name);
        m.clearPass();
        updateMember(m);
    }
}

int SecurityManager::maxAuth(const QString &ip) {
    int max = 0;

    QSqlQuery q;
    q.setForwardOnly(true);

    q.prepare("select auth from trainers where ip=? order by auth desc");
    q.addBindValue(ip);
    q.exec();

    if (q.next()) {
        max = q.value(0).toInt();
    }

    return max;
}

QString SecurityManager::ip(const QString &name)
{
    if (exist(name))
        return member(name).ip;
    else
        return "";
}

void SecurityManager::loadMemberInMemory(const QString &name, QObject *o, const char *slot)
{
    QString n2 = name.toLower();

    if (o == NULL) {
        if (holder.isInMemory(n2))
            return;

        qDebug() << "loading member in memory not with a thread";

        QSqlQuery q;
        q.setForwardOnly(true);
        loadMember(&q, name, GetInfoOnUser);

        return;
    }

    holder.cleanCache();

    WaitingObject *w = WaitingObjects::getObject();

    connect(w, SIGNAL(waitFinished()), o, slot);

    if (holder.isInMemory(n2)) {
        w->emitSignal();
        WaitingObjects::freeObject(w);
    }
    else {
        WaitingObjects::useObject(w);
        connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

        LoadThread *t = getThread();

        t->pushQuery(n2, w, GetInfoOnUser);
    }
}

LoadThread * SecurityManager::getThread()
{
    /* '%' is a safety thing, in case nextLoadThreadNumber is also accessed in writing and that messes it up, at least it isn't out of bounds now */
    int n = nextLoadThreadNumber % loadThreadCount;
    nextLoadThreadNumber = (nextLoadThreadNumber + 1) % loadThreadCount;
    return threads + n;
}

void SecurityManager::insertMember(QSqlQuery *q, void *m2, int update)
{
    SecurityManager::Member *m = (SecurityManager::Member*) m2;

    if (update)
        q->prepare("update trainers set laston=:laston, auth=:auth, banned=:banned, salt=:salt, hash=:hash, ip=:ip where name=:name");
    else
        q->prepare("insert into trainers(name, laston, auth, banned, salt, hash, ip) values(:name, :laston, :auth, :banned, :salt, :hash, :ip)");

    q->bindValue(":name", m->name);
    q->bindValue(":laston", m->date);
    q->bindValue(":auth", m->auth);
    q->bindValue(":banned", m->banned);
    q->bindValue(":hash", m->hash);
    q->bindValue(":salt", m->salt);
    q->bindValue(":ip", m->ip);

    q->exec();
    q->finish();
}

void SecurityManager::loadMember(QSqlQuery *q, const QString &name, int query_type)
{
    if (query_type == SecurityManager::GetInfoOnUser) {
        q->prepare("select laston, auth, banned, salt, hash, ip from trainers where name=? limit 1");
        q->addBindValue(name);
        q->exec();
        if (!q->next()) {
            holder.addNonExistant(name);
        } else {
            SecurityManager::Member m(name, q->value(0).toByteArray(), q->value(1).toInt(), q->value(2).toBool(), q->value(3).toByteArray(),
                                      q->value(4).toByteArray(), q->value(5).toByteArray());
            holder.addMemberInMemory(m);
        }
        q->finish();
    }
}


/* Used for threads */
SecurityManager * SecurityManager::instance = new SecurityManager();

