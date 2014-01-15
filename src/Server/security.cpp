#include <cassert>
#include <ctime>

#include <QSqlQuery>
#include <QSqlRecord>

#include <Utilities/otherwidgets.h>

#include "security.h"
#include "server.h"
#include "waitingobject.h"
#include "loadinsertthread.h"

MemoryHolder<SecurityManager::Member>  SecurityManager::holder;
QNickValidator SecurityManager::val(nullptr);
QHash<QString, unsigned int> SecurityManager::bannedIPs;
QHash<QString, std::pair<QString, int> > SecurityManager::bannedMembers;
int SecurityManager::dailyRunDays = 182;

LoadInsertThread<SecurityManager::Member> * SecurityManager::thread = nullptr;

QFile SecurityManager::memberFile;
int SecurityManager::lastPlace;
istringmap<SecurityManager::Member> SecurityManager::members;
QMultiHash<QString, QString> SecurityManager::playersByIp;
QSet<QString> SecurityManager::authed;

SecurityManager::Member::Member(const QString &name, const QString &date, int auth, bool banned, const QByteArray &salt, const QByteArray &hash,
                                const QString &ip, int ban_expire_time)
    :name(name), date(date), auth(auth), banned(banned), salt(salt), hash(hash), ip(ip), ban_expire_time(ban_expire_time)
{
    filepos = -1;
}

QString SecurityManager::Member::toString() const
{
    char auth[4] = {'0','0','0', '\0'};
    if (this->authority() != 0)
        auth[0] += this->authority();
    if (this->isBanned())
        auth[1] = '1';
    return QString("%1%%2%%3%%4%%5%%6%%7\n").arg(name, date, auth, QString::fromUtf8(salt), QString::fromUtf8(hash), ip, QString::number(ban_expire_time));
}

void SecurityManager::Member::write(QIODevice *device) const {
    assert(filepos >= 0);

    char auth[4] = {'0','0','0', '\0'};
    if (this->authority() != 0 && this->authority() >= 0 && this->authority() <= 9)
        auth[0] += this->authority();
    if (this->isBanned())
        auth[1] = '1';

    device->write(name.toUtf8().constData());
    device->write("%");
    device->write(date.toUtf8().leftJustified(dateLength, ' ', true).constData());
    device->write("%");
    device->write(auth);
    device->write("%");
    device->write(salt.leftJustified(saltLength, ' ', true).constData());
    device->write("%");
    device->write(hash.leftJustified(hashLength, ' ', true).constData());
    device->write("%");
    device->write(ip.leftJustified(ipLength, ' ', true).toUtf8().constData());
    device->write("%");
    device->write(QByteArray::number(ban_expire_time).rightJustified(banTimeLength, '0', true).constData());
    device->write("\n");
}

void SecurityManager::loadSqlMembers() {

    QSqlQuery query;

    query.setForwardOnly(true);

    query.exec("select * from trainers limit 1");

    int count = query.record().count();

    if (count == 8) {
        /* Outdated database, we are going to add ban time */
        QSqlDatabase::database().transaction();

        query.exec("alter table trainers add column ban_expire_time int");
        query.exec("update trainers set ban_expire_time=0");
        //query.exec("create index ban_expire_time_index on trainers (ban_expire_time)");

        QSqlDatabase::database().commit();
    } else if (!query.next()) {
        if (SQLCreator::databaseType == SQLCreator::PostGreSQL) {
            /* The only way to have an auto increment field with PostGreSQL is to my knowledge using the serial type */
            query.exec("create table trainers (id serial, "
                       "name varchar(20), laston char(19), auth int, banned boolean,"
                       "salt varchar(7), hash varchar(32), ip varchar(39), ban_expire_time int, primary key(id), unique(name))");
        } else if (SQLCreator::databaseType == SQLCreator::MySQL) {
            query.exec("CREATE TABLE IF NOT EXISTS trainers (id int(11) NOT NULL auto_increment, "
                       "name varchar(20), laston char(19), auth int(11), banned bool, "
                       "salt varchar(7), hash varchar(32), ip varchar(39), "
                       "ban_expire_time int(11), PRIMARY KEY (id));");
        } else if (SQLCreator::databaseType == SQLCreator::SQLite){
            /* The only way to have an auto increment field with SQLite is to my knowledge having a 'integer primary key' field -- that exact quote */
            query.exec("create table trainers (id integer primary key autoincrement, name varchar(20) unique, "
                       "laston char(19), auth int, banned boolean, salt varchar(7), hash varchar(32), "
                       "ip varchar(39), ban_expire_time int);");
        } else {
            throw QString("Using a not supported database");
        }

        query.exec("create index tname_index on trainers (name)");
        query.exec("create index tip_index on trainers (ip)");

        QFile memberFile("serverdb/members.txt");
        if (memberFile.exists()) {
            Server::print("importing text db");

            if (!memberFile.open(QFile::ReadWrite)) {
                throw QObject::tr("Error: cannot open the file that contains the members ");
            }

            clock_t t = clock();

            query.prepare("insert into trainers(name, laston, auth,  banned, salt, hash, ip, ban_expire_time) values (:name, :laston, :auth,"
                          ":banned, :salt, :hash, :ip, :banexpire)");

            QSqlDatabase::database().transaction();
            int counter = 0;
            while (!memberFile.atEnd()) {
                if (query.lastError().isValid() && counter > 0) {
                    Server::print(QString("Error in last query (number %1): %2").arg(counter).arg(query.lastError().text()));
                    break;
                }

                ++counter;

                if (counter % 1000 == 0) {
                    Server::print(QString("Loaded %1 members so far...").arg(counter));
                }

                QByteArray arr = memberFile.readLine();
                QString s = QString::fromUtf8(arr.constData(), std::max(0,arr.length()-1)); //-1 to remove the \n

                QStringList ls = s.split('%');

                if (ls.size() >= 6 && isValid(ls[0])) {
                    query.bindValue(":name", ls[0].toLower());
                    query.bindValue(":laston",ls[1]);
                    query.bindValue(":auth", ls[2][0].toLatin1()-'0');
                    query.bindValue(":banned", ls[2][1] == '1');
                    /* Weirdly, i seem to have problems when updating something that has a salt containing \, probably postgresql driver,
                       so i remove them. */
                    if (!ls[3].contains('\\')) {
                        query.bindValue(":salt", ls[3].trimmed().toLatin1());
                        query.bindValue(":hash", ls[4].trimmed().toLatin1());
                    } else {
                        query.bindValue(":salt", "");
                        query.bindValue(":hash", "");
                    }
                    query.bindValue(":ip", ls[5].trimmed());
                    if (ls.size() >= 7) {
                        query.bindValue(":banexpire", ls[6]);
                    } else {
                        query.bindValue(":banexpire", 0);
                    }
                    query.exec();
                }
            }

            QSqlDatabase::database().commit();

            t = clock() - t;

            Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
            Server::print(query.lastError().text());
        }
    }

    /* Expire old temp bans */
    if (SQLCreator::databaseType == SQLCreator::MySQL) {
        query.prepare("update trainers set banned=0 where banned=1 and ban_expire_time < :now and ban_expire_time != 0");
    } else {
        query.prepare("update trainers set banned='false' where banned='true' and ban_expire_time < :now and ban_expire_time != 0");
    }
    query.bindValue(":now", QDateTime::currentDateTimeUtc().toTime_t());
    query.exec();
    QSqlDatabase::database().commit();

    /* Loading the ban list */

    if (SQLCreator::databaseType == SQLCreator::MySQL) {
        query.exec("select name, ip, ban_expire_time from trainers where banned=1");
    }
    else {
        query.exec("select name, ip, ban_expire_time from trainers where banned='true'");
    }

    while (query.next()) {
        bannedIPs.insert(query.value(1).toString(), query.value(2).toInt());
        bannedMembers.insert(query.value(0).toString().toLower(), std::make_pair(query.value(1).toString(), query.value(2).toInt()));
    }
}

void SecurityManager::loadMembers()
{
    if (isSql()) {
        loadSqlMembers();
        return;
    }
    const char *path = "serverdb/members.txt";
    const char *backup = "serverdb/members.backup.txt";
    {
        QDir d;
        d.mkdir("serverdb");
    }

    if (!QFile::exists(path) && QFile::exists(backup)) {
        QFile::rename(backup, path);
    }

    memberFile.setFileName(path);
    if (!memberFile.open(QFile::ReadWrite)) {
        throw QObject::tr("Error: cannot open the file that contains the members (%1)").arg(path);
    }

    int pos = memberFile.pos();
    while (!memberFile.atEnd()) {
        QByteArray arr = memberFile.readLine();
        QString s = QString::fromUtf8(arr.constData(), std::max(0,arr.length()-1)); //-1 to remove the \n

        QStringList ls = s.split('%');

        if (ls.size() >= 6 && isValid(ls[0])) {
            Member m (ls[0], ls[1].trimmed(), ls[2][0].toLatin1() - '0', ls[2][1] == '1', ls[3].trimmed().toLatin1(), ls[4].trimmed().toLatin1(), ls[5].trimmed());

            if (ls.size() >= 7) {
                m.ban_expire_time = ls[6].toInt();
            }

            m.filepos = pos;
            members[ls[0]] = m;

            /* Update pos for next iteration */
            pos = memberFile.pos();

            if (m.isBanned()) {
                bannedIPs.insert(m.ip, m.ban_expire_time);
                bannedMembers.insert(m.name.toLower(), std::pair<QString, int>(m.ip, m.ban_expire_time));
            }
            if (m.authority() > 0) {
                authed.insert(m.name);
            }
            playersByIp.insert(m.ip, m.name);
        }
        lastPlace = memberFile.pos();
    }

    //We also clean up the file by rewritting it with only the valid contents
    QFile temp (backup);
    if (!temp.open(QFile::WriteOnly | QFile::Truncate))
        throw QObject::tr("Impossible to change %1").arg(backup);

    pos = temp.pos();

    for(auto it = members.begin(); it != members.end(); ++it) {
        Member &m = it->second;
        m.write(&temp);
        m.filepos = pos;
        pos = temp.pos();
    }

    lastPlace = temp.pos();

    temp.flush();
    memberFile.remove();

    if (!temp.rename(path)) {
        throw QObject::tr("Error: cannot rename the file that contains the members (%1 -> %2)").arg(backup).arg(path);
    }

    temp.rename(path);

    if (!memberFile.open(QFile::ReadWrite)) {
        throw QObject::tr("Error: cannot reopen the file that contains the members (%1)").arg(path);
    }
}

void SecurityManager::init()
{
    thread = new LoadInsertThread<Member>;

    connect(thread, SIGNAL(processLoad(QSqlQuery*, QVariant, int, WaitingObject*)), instance, SLOT(loadMember(QSqlQuery*, QVariant,int)), Qt::DirectConnection);
    connect(thread, SIGNAL(processWrite(QSqlQuery*, void*,int)), instance, SLOT(insertMember(QSqlQuery*, void*,int)), Qt::DirectConnection);
    connect(thread, SIGNAL(processDailyRun(QSqlQuery*)), instance, SLOT(dailyRunEx(QSqlQuery*)));

    thread->start();

    loadMembers();
}

void SecurityManager::destroy()
{
    thread->finish();
}

bool SecurityManager::isValid(const QString &name) {
    return val.validate(name) == QValidator::Acceptable;
}

bool SecurityManager::exist(const QString &name)
{
    if (!holder.isInMemory(name)) {
        loadMemberInMemory(name);
    }

    if (isSql()) {
        return holder.exists(name);
    } else {
        return members.find(name) != members.end();
    }
}

SecurityManager::Member SecurityManager::member(const QString &name)
{
    if (!holder.isInMemory(name)) {
        loadMemberInMemory(name);
    }

    assert(exist(name));

    if (isSql()) {
        return holder.member(name);
    } else {
        return members[name];
    }
}

QStringList SecurityManager::membersForIp(const QString &ip)
{
    if (isSql()) {
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
    return playersByIp.values(ip);
}

QHash<QString, std::pair<QString, int> > SecurityManager::banList()
{
    return bannedMembers;
}

QStringList SecurityManager::authList()
{
    if (isSql()) {
        QSqlQuery q;
        q.setForwardOnly(true);

        q.exec("select name from trainers where auth>0");

        QStringList ret;
        while (q.next()) {
            ret.push_back(q.value(0).toString());
        }

        return ret;
    }
    return authed.toList();
}

QStringList&& SecurityManager::userList()
{
    QStringList ret;

    if (isSql()) {
        QSqlQuery q;
        q.setForwardOnly(true);

        q.exec("select name from trainers");

        QStringList ret;
        while (q.next()) {
            ret.push_back(q.value(0).toString());
        }
    } else {
        for(auto it = members.begin(); it != members.end(); ++it) {
            ret.push_back(it->first);
        }
    }

    return std::move(ret);
}

void SecurityManager::deleteUser(const QString &name)
{
    if (isSql()) {
        QSqlQuery q;
        q.setForwardOnly(true);
        q.prepare("delete from trainers where name=:name");
        q.bindValue(":name", name.toLower());
        q.exec();
        return;
    }

    Member m = member(name);

    assert (m.name.length() > 0);

    m.name[0] = ':'; //invalid character, will not get loaded when reloading database
    memberFile.seek(m.filepos);
    members[name].write(&memberFile);
    memberFile.flush();

    playersByIp.remove(m.ip, name);
    authed.remove(name);
    members.erase(name);
    bannedMembers.remove(name.toLower());

    if (bannedIPs.contains(m.ip) && !playersByIp.contains(m.ip)) {
        bannedIPs.remove(m.ip);
    }
}

void SecurityManager::create(const QString &name, const QString &date, const QString &ip, bool banned) {
    Member m(name.toLower(), date, 0, banned, "", "", ip, 0);
    holder.addMemberInMemory(m);
    updateMemberInDatabase(m, true);
}

void SecurityManager::updateMemberInDatabase(const Member &m, bool add)
{
    bool update = !add;

    if (isSql()) {
        thread->pushMember(m, update);
    } else {
        /* Can't write in a threaded manner, because can't update memory in a threaded manner and update filepos in a threaded
         * manner. Could with mutexes */
        Member m2 = m;
        insertMember(0, &m2, update);
    }
}

void SecurityManager::updateMember(const Member &m) {
    holder.addMemberInMemory(m);

    updateMemberInDatabase(m, false);
}

bool SecurityManager::bannedIP(const QString &ip) {
    QHash<QString, unsigned int>::const_iterator i = bannedIPs.find(ip);
    bool isBanned = (i != bannedIPs.end());
    if (isBanned && i.value() != 0 && i.value() < qlonglong(QDateTime::currentDateTimeUtc().toTime_t())) {
        /* We expire the tempban here if we should */
        unbanIP(ip);
        return false;
    }
    return isBanned;
}

void SecurityManager::ban(const QString &name, int time) {
    if (exist(name)) {
        banIP(member(name).ip, time);
    }
}

void SecurityManager::banIP(const QString &ip, int time)
{
    if (bannedIPs.contains(ip)) {
        return;
    }

    if (time <= 0) {
        bannedIPs.insert(ip, 0);
    } else {
        bannedIPs.insert(ip, QDateTime::currentDateTimeUtc().toTime_t() + time * 60);
    }

    QStringList aliases = membersForIp(ip);

    if (aliases.length() > 0) {
        foreach(const QString &name, aliases) {
            Member m = member(name);
            m.ban(time);

            bannedMembers.insert(name.toLower(), std::make_pair(m.ip, m.ban_expire_time));

            updateMember(m);
        }
    } else {
        create(ip,QDateTime::currentDateTime().toString(Qt::ISODate),ip);

        Member m = member(ip);
        m.ban(time);

        bannedMembers.insert(m.name.toLower(), std::make_pair(m.ip, m.ban_expire_time));

        updateMember(m);
    }
}

void SecurityManager::unban(const QString &name) {
    if (exist(name)) {
        Member m = member(name);
        unbanIP(m.ip);
    }
}

bool SecurityManager::registered(const QString &name)
{
    return member(name).isProtected();
}

int SecurityManager::auth(const QString &name)
{
    return member(name).authority();
}

int SecurityManager::numRegistered(const QString &ip)
{
    if (isSql()) {
        QSqlQuery q;
        q.setForwardOnly(true);

        if (SQLCreator::databaseType == SQLCreator::SQLite) {
            /* On SQLite, there's some bug with the qt driver probably,
                    but here it oftens return nothing if i use '=' instead of 'like', so... */
            q.prepare("select count(*) from trainers where length(hash) > 0 and ip like ?");
        } else {
            q.prepare("select count(*) from trainers where length(hash) > 0 and ip=?");
        }

        q.addBindValue(ip);
        q.exec();

        q.next();
        return q.value(0).toInt();
    }

    int ret = 0;

    foreach(const QString &name, membersForIp(ip)) {
        ret += registered(name);
    }

    return ret;
}

void SecurityManager::unbanIP(const QString &ip)
{
    QList<QString> _members = membersForIp(ip);

    foreach(QString name, _members)
    {
        Member m = member(name);
        m.unban();
        bannedMembers.remove(name.toLower());
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

//void SecurityManager::setBanExpireTime(const QString &name, int time) {
//    if (exist(name)) {
//        Member m = member(name);
//        m.ban_expire_time = time;
//        updateMember(m);
//    }
//}


int SecurityManager::maxAuth(const QString &ip) {
    int max = 0;

    foreach(QString name, playersByIp.values(ip)) {
        max = std::max(auth(name), max);
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

    if (!o) {
        if (holder.isInMemory(n2))
            return;

        qDebug() << "loading member in memory not with a thread";

        assert(isSql());

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
        auto *t = getThread();

        t->pushQuery(n2, w, GetInfoOnUser);
    }
}

LoadInsertThread<SecurityManager::Member> * SecurityManager::getThread()
{
    return thread;
}

void SecurityManager::insertMember(QSqlQuery *q, void *m2, int update)
{
    SecurityManager::Member &m = * (SecurityManager::Member*) m2;

    if (isSql()) {
        if (update)
            q->prepare("update trainers set laston=:laston, auth=:auth, banned=:banned, salt=:salt, hash=:hash, ip=:ip, ban_expire_time=:banexpire where name=:name");
        else
            q->prepare("insert into trainers(name, laston, auth, banned, salt, hash, ip, ban_expire_time) values(:name, :laston, :auth, :banned, :salt, :hash, :ip, :banexpire)");

        q->bindValue(":name", m.name.toLower());
        q->bindValue(":laston", m.date);
        q->bindValue(":auth", m.auth);
        q->bindValue(":banned", m.banned);
        q->bindValue(":hash", m.hash);
        q->bindValue(":salt", m.salt);
        q->bindValue(":ip", m.ip);
        q->bindValue(":banexpire", m.ban_expire_time);

        q->exec();
        q->finish();

        return;
    }

    if (update) {
        Member oldm = member(m.name);
        m.filepos = oldm.filepos;

        playersByIp.remove(oldm.ip, oldm.name);
        authed.remove(oldm.name);

        members[m.name] = m;
        memberFile.seek(m.filepos);
        m.write(&memberFile);
        memberFile.flush();
    } else {
        m.filepos = lastPlace;
        members[m.name] = m;

        memberFile.seek(lastPlace);
        m.write(&memberFile);

        lastPlace = memberFile.pos();

        memberFile.flush();
    }

    playersByIp.insert(m.ip,m.name);
    if (m.auth > 0) {
        authed.insert(m.name);
    }
}

void SecurityManager::loadMember(QSqlQuery *q, const QVariant &name, int query_type)
{
    if (!isSql()) {
        /* No SQL usage, all already in memory */
        return;
    }

    if (query_type == SecurityManager::GetInfoOnUser) {
        q->prepare("select laston, auth, banned, salt, hash, ip, ban_expire_time from trainers where name=:name limit 1");
        q->bindValue(":name", name.toString().toLower());
        q->exec();
        if (!q->next()) {
            holder.addNonExistant(name.toString().toLower());
        } else {
            Member m(name.toString().toLower(), q->value(0).toString(), q->value(1).toInt(), q->value(2).toBool(), q->value(3).toByteArray(),
                     q->value(4).toByteArray(), q->value(5).toString(), q->value(6).toInt());
            holder.addMemberInMemory(m);
        }
        q->finish();
    }
}

void SecurityManager::exportDatabase()
{
    if (!isSql()) {
        return;
    }

    {
        QDir d;
        d.mkdir("serverdb");
    }

    QFile out("serverdb/members.txt");

    out.open(QIODevice::WriteOnly);

    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec("select name, laston, auth, banned, salt, hash, ip, ban_expire_time from trainers order by name asc");

    while (q.next()) {
        Member m(q.value(0).toString(), q.value(1).toString(), q.value(2).toInt(), q.value(3).toBool(), q.value(4).toByteArray(), q.value(5).toByteArray()
                 , q.value(6).toString(), q.value(7).toInt());
        out.write(m.toString().toUtf8());
    }

    Server::print("Member database exported!");
}

void SecurityManager::processDailyRun(int maxdays, bool async)
{
    qDebug() << "Set daily run days to " << maxdays;
    dailyRunDays = maxdays;
    if (async) {
        thread->addDailyRun();
    } else {
        if (isSql()) {
            QSqlQuery q;
            dailyRunEx(&q);
        } else {
            dailyRunEx();
        }
    }
}

const istringmap<SecurityManager::Member> & SecurityManager::getMembers() {
    if (isSql()) {
        members.clear();

        QSqlQuery q;
        q.setForwardOnly(true);
        q.exec("select name, auth, banned, hash, ip, laston, ban_expire_time from trainers order by name asc");

        while(q.next()) {
            Member m(q.value(0).toString(), q.value(5).toString(), q.value(1).toInt(), q.value(2).toBool(), q.value(3).toByteArray(), q.value(3).toByteArray(), q.value(4).toString(), q.value(6).toInt());

            members[m.name] = m;
        }
    }

    return members;
}

void SecurityManager::dailyRunEx(QSqlQuery *q)
{
    if (q) {
        QString limit = QDateTime::currentDateTime().addDays(-dailyRunDays).toString(Qt::ISODate);

        qDebug() << "Processing daily run for members with limit " << limit;
        if (SQLCreator::databaseType == SQLCreator::MySQL) {
            q->prepare("delete from trainers where laston<? and auth=0 and banned=0");
        } else {
            q->prepare("delete from trainers where laston<? and auth=0 and banned='false'");
        }
        q->addBindValue(limit);

        q->exec();
        q->finish();
        qDebug() << "Daily run for members finished";

        return;
    }

    QString limit = QDateTime::currentDateTime().addDays(-dailyRunDays).toString(Qt::ISODate);

    qDebug() << "Processing daily run for members with limit " << limit;

    QStringList toDelete;
    for (auto it = members.begin(); it != members.end(); ++it) {
        Member &m = it->second;

        if (m.authority() <= 0 && !m.isBanned() && m.date < limit) {
            toDelete.push_back(m.name);
        }
    }

    foreach(QString name, toDelete) {
        deleteUser(name);
    }

    qDebug() << "Daily run for members finished";
}

/* Used for threads */
SecurityManager * SecurityManager::instance = new SecurityManager();

