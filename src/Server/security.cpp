#include <cassert>
#include <ctime>

#include "../Utilities/otherwidgets.h"

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


void SecurityManager::loadMembers()
{
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

    connect(thread, SIGNAL(processLoad(QVariant, int, WaitingObject*)), instance, SLOT(loadMember(QVariant,int)), Qt::DirectConnection);
    connect(thread, SIGNAL(processWrite(void*,int)), instance, SLOT(insertMember(void*,int)), Qt::DirectConnection);
    connect(thread, SIGNAL(processDailyRun()), instance, SLOT(dailyRunEx()));

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

    //return holder.exists(name);
    return members.find(name) != members.end();
}

SecurityManager::Member SecurityManager::member(const QString &name)
{
    if (!holder.isInMemory(name)) {
        loadMemberInMemory(name);
    }

    //return holder.member(name);

    assert(exist(name));

    return members[name];
}


QStringList SecurityManager::membersForIp(const QString &ip)
{
    return playersByIp.values(ip);
}

QHash<QString, std::pair<QString, int> > SecurityManager::banList()
{
    return bannedMembers;
}

QStringList SecurityManager::authList()
{
    return authed.toList();
}

QStringList SecurityManager::userList()
{
    QStringList ret;
    for(auto it = members.begin(); it != members.end(); ++it) {
        ret.push_back(it->first);
    }

    return ret;
}

void SecurityManager::deleteUser(const QString &name)
{
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

    /* Can't write in a thread manner, because can't update memory in a threaded manner and update filepos in a threaded
     * manner. Could with mutexes */
#if 0
    thread->pushMember(m, update);
#else
    Member m2 = m;
    insertMember(&m2, update);
#endif
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
    return members.at(name).isProtected();
}

int SecurityManager::auth(const QString &name)
{
    return members.at(name).authority();
}

int SecurityManager::numRegistered(const QString &ip)
{
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

        loadMember(n2, GetInfoOnUser);

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

void SecurityManager::insertMember(void *m2, int update)
{
    SecurityManager::Member &m = * (SecurityManager::Member*) m2;

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

void SecurityManager::loadMember(const QVariant &name, int query_type)
{
    (void) name;
    (void) query_type;

    return;

    /* No SQL usage, all already in memory */
#if 0
    if (query_type == SecurityManager::GetInfoOnUser) {
        q->prepare("select laston, auth, banned, salt, hash, ip, ban_expire_time from trainers where name=:name limit 1");
        q->bindValue(":name", name);
        q->exec();
        if (!q->next()) {
            holder.addNonExistant(name.toString());
        } else {
            Member m(name.toString(), q->value(0).toString(), q->value(1).toInt(), q->value(2).toBool(), q->value(3).toByteArray(),
                                      q->value(4).toByteArray(), q->value(5).toString(), q->value(6).toInt());
            holder.addMemberInMemory(m);
        }
        q->finish();
    }
#endif
}

void SecurityManager::processDailyRun(int maxdays, bool async)
{
    qDebug() << "Set daily run days to " << maxdays;
    dailyRunDays = maxdays;
    if (async) {
        thread->addDailyRun();
    } else {
        dailyRunEx();
    }
}

const istringmap<SecurityManager::Member> & SecurityManager::getMembers() {
    return members;
}

void SecurityManager::dailyRunEx()
{
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

