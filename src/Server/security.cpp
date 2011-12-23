#include "security.h"
#include "sql.h"
#include <QtSql>
#include "../Utilities/otherwidgets.h"
#include <ctime>
#include "server.h"
#include "waitingobject.h"
#include "loadinsertthread.h"

MemoryHolder<SecurityManager::Member>  SecurityManager::holder;
QNickValidator SecurityManager::val(NULL);
QSet<QString> SecurityManager::bannedIPs;
QHash<QString, QString> SecurityManager::bannedMembers;
int SecurityManager::nextLoadThreadNumber = 0;
LoadThread ** SecurityManager::threads = NULL;
InsertThread<SecurityManager::Member> * SecurityManager::ithread = NULL;

SecurityManager::Member::Member(const QString &name, const QByteArray &date, int auth, bool banned, const QByteArray &salt, const QByteArray &hash,
                                const QByteArray &ip)
    :name(name.toLower()), date(date), auth(auth), banned(banned), salt(salt), hash(hash), ip(ip)
{
}

QString SecurityManager::Member::toString() const
{
    char auth[4] = {'0','0','0', '\0'};
    if (this->authority() != 0)
        auth[0] += this->authority();
    if (this->isBanned())
        auth[1] = '1';
    return QString("%1%%2%%3%%4%%5%%6\n").arg(name, date, auth, salt, hash, ip);
}

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

        QSqlDatabase::database().transaction();
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

        QSqlDatabase::database().commit();

        t = clock() - t;

        Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
        Server::print(query.lastError().text());
    }

    /* Loading the ban list */

    if (SQLCreator::databaseType == SQLCreator::MySQL) {
        query.exec("select name, ip from trainers where banned=1");
    }
        else {
        query.exec("select name, ip from trainers where banned='true'");
    }

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

void SecurityManager::init()
{
    threads = new LoadThread*[loadThreadCount];

    for (int i = 0; i < loadThreadCount; i++) {
        threads[i] = new LoadThread();
        connect(threads[i], SIGNAL(processQuery (QSqlQuery *, QVariant, int, WaitingObject*)), instance, SLOT(loadMember(QSqlQuery*,QVariant,int)), Qt::DirectConnection);
        threads[i]->start();
    }

    ithread = new InsertThread<Member>();
    connect(ithread, SIGNAL(processMember(QSqlQuery*,void*,int)), instance, SLOT(insertMember(QSqlQuery*,void*,int)), Qt::DirectConnection);

    ithread->start();


    loadMembers();
}

void SecurityManager::destroy()
{
    ithread->finish();
    for (int i = 0; i < loadThreadCount; i++) {
        threads[i]->finish();
    }
    delete [] threads;
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

    if (SQLCreator::databaseType == SQLCreator::SQLite) {
        /* On SQLite, there's some bug with the qt driver probably,
           but here it oftens return nothing if i use '=' instead of 'like', so... */
        q.prepare("select name from trainers where ip like ?");
    } else {
        q.prepare("select name from trainers where ip=?");
    }
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

QStringList SecurityManager::authList()
{
    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec("select name from trainers where auth>0");

    QStringList ret;
    while (q.next()) {
        ret.push_back(q.value(0).toString());
    }

    return ret;
}
QStringList SecurityManager::userList()
{
    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec("select name from trainers");

    QStringList ret;
    while (q.next()) {
        ret.push_back(q.value(0).toString());
    }

    return ret;
}

void SecurityManager::deleteUser(const QString &name)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("delete from trainers where name=?");
    q.addBindValue(name);
    q.exec();
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

void SecurityManager::setAuth(const QString &name, int auth) {
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

    if (SQLCreator::databaseType != SQLCreator::SQLite) {
        q.prepare("select auth from trainers where ip=? order by auth desc");
    } else {
        q.prepare("select auth from trainers where ip like ? order by auth desc");
    }
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
        loadMember(&q, n2, GetInfoOnUser);

        return;
    }

    holder.cleanCache();

    WaitingObject *w = WaitingObjects::getObject();

    connect(w, SIGNAL(waitFinished()), o, slot);
    connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

    if (holder.isInMemory(n2)) {
        w->emitSignal();
    }
    else {
        LoadThread *t = getThread();

        t->pushQuery(n2, w, GetInfoOnUser);
    }
}

LoadThread * SecurityManager::getThread()
{
    /* '%' is a safety thing, in case nextLoadThreadNumber is also accessed in writing and that messes it up, at least it isn't out of bounds now */
    int n = nextLoadThreadNumber % loadThreadCount;
    nextLoadThreadNumber = (nextLoadThreadNumber + 1) % loadThreadCount;
    return threads[n];
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

void SecurityManager::loadMember(QSqlQuery *q, const QVariant &name, int query_type)
{
    if (query_type == SecurityManager::GetInfoOnUser) {
        q->prepare("select laston, auth, banned, salt, hash, ip from trainers where name=? limit 1");
        q->addBindValue(name);
        q->exec();
        if (!q->next()) {
            holder.addNonExistant(name.toString());
        } else {
            Member m(name.toString(), q->value(0).toByteArray(), q->value(1).toInt(), q->value(2).toBool(), q->value(3).toByteArray(),
                                      q->value(4).toByteArray(), q->value(5).toByteArray());
            holder.addMemberInMemory(m);
        }
        q->finish();
    }
}

void SecurityManager::exportDatabase()
{
    QFile out("members.txt");

    out .open(QIODevice::WriteOnly);

    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec("select name, laston, auth, banned, salt, hash, ip from trainers order by name asc");

    while (q.next()) {
        Member m(q.value(0).toString(), q.value(1).toByteArray(), q.value(2).toInt(), q.value(3).toBool(), q.value(4).toByteArray(), q.value(5).toByteArray()
                 , q.value(6).toByteArray());
        out.write(m.toString().toUtf8());
    }

    Server::print("Member database exported!");
}


/* Used for threads */
SecurityManager * SecurityManager::instance = new SecurityManager();

