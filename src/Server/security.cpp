#include "security.h"
#include "../Utilities/otherwidgets.h"
#include <cmath>

QMap<QString, SecurityManager::Member> SecurityManager::members;
QNickValidator SecurityManager::val(NULL);
QSet<QString> SecurityManager::bannedIPs;
QSet<QString> SecurityManager::bannedMembers;
QMultiMap<QString, QString> SecurityManager::playersByIp;
const char * SecurityManager::path = "members.txt";
QHash<QString, int> SecurityManager::memberPlaces;
QFile SecurityManager::memberFile(SecurityManager::path);
int SecurityManager::lastPlace;

void SecurityManager::loadMembers()
{
    if (!QFile::exists(path) && QFile::exists("users.tmp"))
        QFile::rename("users.tmp", "members.txt");

    if (!memberFile.open(QFile::ReadWrite)) {
        throw QObject::tr("Error: cannot open the file that contains the members (%1)").arg(path);
    }

    int pos = memberFile.pos();
    while (!memberFile.atEnd()) {
        QByteArray arr = memberFile.readLine();
        QString s = QString::fromUtf8(arr.constData(), std::max(0,arr.length()-1)); //-1 to remove the \n

        QStringList ls = s.split('%');

        if (ls.size() == 7 && isValid(ls[0])) {
            Member m (ls[0], ls[1], ls[2], ls[3].toInt(), ls[4], ls[5], ls[6]);
            members[ls[0]] = m;
            memberPlaces[m.name] = pos;
            pos = memberFile.pos();

            if (m.isBanned()) {
                bannedIPs.insert(m.ip.trimmed());
                bannedMembers.insert(m.name);
            }
            playersByIp.insert(m.ip.trimmed(), m.name);
        }
        lastPlace = memberFile.pos();
    }

    //We also clean up the file by rewritting it with only the valid contents
    QFile temp ("users.tmp");
    if (!temp.open(QFile::WriteOnly | QFile::Truncate))
        throw QObject::tr("Impossible to change users.tmp");

    pos = temp.pos();
    foreach(Member m, members) {
        m.write(&temp);
        memberPlaces[m.name] = pos;
        pos = temp.pos();
    }

    lastPlace = temp.pos();

    temp.flush();
    memberFile.remove();
    temp.rename(path);

    if (!memberFile.open(QFile::ReadWrite)) {
        throw QObject::tr("Error: cannot reopen the file that contains the members (%1)").arg(path);
    }
}

SecurityManager::Member::Member(const QString &name, const QString &date, const QString &auth, int ladder, const QString &salt, const QString &hash, const QString &ip)
    :name(name.toLower()), date(date), auth(auth.leftJustified(3,'0',true)), salt(salt.leftJustified(saltLength)),
    hash(hash.leftJustified(hashLength)), ip(ip.leftJustified(ipLength,' ',true)), ladder(ladder)
{
}

void SecurityManager::Member::write(QIODevice *device) const {
    device->write(name.toUtf8().constData());
    device->write("%");
    device->write(date.toUtf8().constData());
    device->write("%");
    device->write(auth.toUtf8().constData());
    device->write("%");
    device->write(QByteArray::number(ladder).rightJustified(ladderLength, '0', true).constData());
    device->write("%");
    device->write(salt.toUtf8().constData());
    device->write("%");
    device->write(hash.toUtf8().constData());
    device->write("%");
    device->write(ip.toUtf8().constData());
    device->write("\n");
}

void SecurityManager::Member::changeRating(int opponent_rating, bool win)
{
    int n = auth[2].toAscii()-'0';

    int newrating;

    if (n == 0) {
        if (win) {
            newrating = std::min(opponent_rating+100, 1250);
        } else {
            newrating = std::max(opponent_rating-100, 750);
        }
    } else {
        int kfactor;
        if (n <= 5) {
            static const int kfactors[] = {200, 150, 100, 80, 65, 50};
            kfactor = kfactors[n];
        } else {
            kfactor = 32;
        }
        double myesp = 1/(1+ pow(10., (opponent_rating-rating())/400));
        double result = win;

        newrating = rating() + (result - myesp)*kfactor;
    }

    if (n <= 5) {
        auth[2]='0'+n+1;
    }

    ladder = newrating;
    SecurityManager::updateMember(*this);
}

void SecurityManager::init()
{
    loadMembers();
}

bool SecurityManager::isValid(const QString &name) {
    return val.validate(name) == QValidator::Acceptable;
}

bool SecurityManager::exist(const QString &name)
{
    return members.contains(name.toLower());
}

QMap<QString,SecurityManager::Member> SecurityManager::getMembers() {
    return members;
}

QList<QString> SecurityManager::membersForIp(const QString &ip)
{
    return playersByIp.values(ip);
}

QSet<QString> SecurityManager::banList()
{
    return bannedMembers;
}

void SecurityManager::create(const Member &m) {
    members[m.name] = m;

    memberFile.seek(lastPlace);
    m.write(&memberFile);
    memberPlaces[m.name] = lastPlace;
    playersByIp.insert(m.ip.trimmed(),m.name);
    lastPlace = memberFile.pos();

    memberFile.flush();
}

void SecurityManager::updateMember(const Member &m) {
    updateMemory(m);

    memberFile.seek(memberPlaces[m.name]);
    members[m.name].write(&memberFile);
    memberFile.flush();
}


void SecurityManager::updateMemory(const Member &m) {
    playersByIp.remove(members[m.name].ip.trimmed(),m.name);
    members[m.name] = m;
    playersByIp.insert(m.ip.trimmed(), m.name);
}

bool SecurityManager::bannedIP(const QString &ip) {
    foreach(QString s, bannedIPs) {
        qDebug() << "banned: " << s;
    }

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

    QStringList l = playersByIp.values(ip);

    foreach(QString name, l) {
        max = std::max(max, member(name).authority());
    }

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
