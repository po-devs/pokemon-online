#include "security.h"
#include "../Utilities/otherwidgets.h"

QHash<QString, SecurityManager::Member> SecurityManager::members;
QNickValidator SecurityManager::val(NULL);
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

        qDebug() << "Size:" << ls.size() << ", length: " << arr.length() << " "  << s.length();

        if (ls.size() == 5 && isValid(ls[0])) {
            Member m (ls[0], ls[1], ls[2], ls[3], ls[4]);
            members[ls[0]] = m;
            memberPlaces[m.name] = pos;
            pos = memberFile.pos();
        }
        lastPlace = memberFile.pos();
    }

    //We also clean up the file by rewritting it with only the valid contents
    QFile temp ("users.tmp");
    if (!temp.open(QFile::WriteOnly | QFile::Truncate))
        throw QObject::tr("Impossible to change users.tmp");

    temp.pos();
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

SecurityManager::Member::Member(const QString &name, const QString &date, const QString &salt, const QString &hash, const QString &ip)
    :name(name), date(date), salt(salt.leftJustified(saltLength)), hash(hash.leftJustified(hashLength)), ip(ip.leftJustified(ipLength))
{
}

void SecurityManager::Member::write(QIODevice *device) const {
    device->write(name.toUtf8().constData());
    device->write("%");
    device->write(date.toUtf8().constData());
    device->write("%");
    device->write(salt.toUtf8().constData());
    device->write("%");
    device->write(hash.toUtf8().constData());
    device->write("%");
    device->write(ip.toUtf8().constData());
    device->write("\n");
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
    return members.contains(name);
}

void SecurityManager::create(const Member &m) {
    members[m.name] = m;

    memberFile.seek(lastPlace);
    m.write(&memberFile);
    memberPlaces[m.name] = lastPlace;
    lastPlace = memberFile.pos();

    memberFile.flush();
}

void SecurityManager::updateMember(const Member &m) {
    members[m.name] = m;

    memberFile.seek(memberPlaces[m.name]);
    members[m.name].write(&memberFile);
}


void SecurityManager::updateMemory(const Member &m) {
    members[m.name] = m;
}

