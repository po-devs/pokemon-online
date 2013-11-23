#include <QDebug>
#include <QSettings>
#include "antidos.h"

AntiDos::AntiDos(QSettings &settings) {
    loadVals(settings);
    // Clears history every day, to save RAM.
    connect(&timer, SIGNAL(timeout()), this, SLOT(clearData()));
}

AntiDos * AntiDos::instance = nullptr;

void AntiDos::init(QSettings &settings)
{
    instance = new AntiDos(settings);
}

void AntiDos::loadVals(QSettings &settings) {
    trusted_ips = settings.value("AntiDOS/TrustedIps").toString().split(QRegExp("\\s*,\\s*"));
    max_people_per_ip = settings.value("AntiDOS/MaxPeoplePerIp").toInt();
    max_commands_per_user = settings.value("AntiDOS/MaxCommandsPerUser").toInt();
    max_kb_per_user = settings.value("AntiDOS/MaxKBPerUser").toInt();
    max_login_per_ip = settings.value("AntiDOS/MaxConnectionRatePerIP").toInt();
    ban_after_x_kicks = settings.value("AntiDOS/NumberOfInfractionsBeforeBan").toInt();
    notificationsChannel = settings.value("AntiDOS/NotificationsChannel").toString();
    on = !settings.value("AntiDOS/Disabled").toBool();
}

bool AntiDos::connecting(const QString &ip)
{
    if (loginsPerIp.contains(ip)) {
        QList<time_t> &l = loginsPerIp[ip];

        int i = 0;

        /* Removing connections older than 1 minute */
        while (i < l.size()) {
            if (time(NULL)-l[i] > 60) {
                i++;
            }  else {
                break;
            }
        }

        l.erase(l.begin(), l.begin()+i);

        if (l.size() >= max_login_per_ip && on) {
            //qDebug() << "Too many attempts for IP " << ip;
            return false;
        }
    }

    if (connectionsPerIp.value(ip) >= max_people_per_ip && on) {
        /* That way it won't appear in the logs if they spam DoS connections */
        if (rand() % 3)
            loginsPerIp[ip].push_back(time(NULL));
        qDebug() << "Too many people for IP " << ip;
        return false;
    }


    /* Registering the connection */
    loginsPerIp[ip].push_back(time(NULL));
    connectionsPerIp[ip]++;
    //Server::serverIns->printLine(tr("Connections for ip(+conn) %1 are %2").arg(ip).arg(connectionsPerIp[ip]));

    return true;
}

void AntiDos::disconnect(const QString &ip, int id)
{
    connectionsPerIp[ip]--;
    //Server::serverIns->printLine(tr("Connections for ip(-disc) %1 are %2").arg(ip).arg(connectionsPerIp[ip]));
    transfersPerId.remove(id);
    sizeOfTransfers.remove(id);
    if (connectionsPerIp[ip]==0) {
        connectionsPerIp.remove(ip);
    }
}

bool AntiDos::changeIP(const QString &newIp, const QString &oldIp)
{
    connectionsPerIp[oldIp]--;
    //Server::serverIns->printLine(tr("Connections for ip(-change) %1 are %2").arg(oldIp).arg(connectionsPerIp[oldIp]));
    loginsPerIp[oldIp].pop_back(); // remove a login
    if (connectionsPerIp[oldIp] <= 0) {
        connectionsPerIp.remove(oldIp);
    }
    return connecting(newIp);
}


int AntiDos::numberOfDiffIps()
{
    return connectionsPerIp.count();
}

bool AntiDos::transferBegin(int id, int length, const QString &ip)
{
    if (id < 0) {
        qFatal("Fatal! Negative id in AntiDOS: %d", id);
    }

    /* If the IP is in the Trusted Ips list, do not do anything */
    if (trusted_ips.contains(ip)) {
        return true;
    }

    if (transfersPerId.contains(id)) {
        QList< QPair<time_t, size_t> > &l = transfersPerId[id];
        int &len = sizeOfTransfers[id];
        int orlen = len;

        int i = 0;

        /* Removing commands older than 1 minute */
        while (i < l.size()) {
            if (time(NULL)-l[i].first > 60) {
                len -= l[i].second;
                i++;
            }  else {
                break;
            }
        }

        if (len < 0) {
            qFatal("Fatal! Negative length in antidos: %d > %d, id: %d, removed: %d", orlen, len, id, i);
        }

        l.erase(l.begin(), l.begin()+i);


        if (l.size() >= max_commands_per_user && on) {
            emit kick(id);
            addKick(ip);
            return false;
        }


        if (len + length > max_kb_per_user*1024 && on) {
            emit kick(id);
            addKick(ip);
            return false;
        }
    } else if (length > max_kb_per_user*1024 && on) {
        emit kick(id);
        addKick(ip);
        return false;
    }

    sizeOfTransfers[id] += length;
    transfersPerId[id].push_back(QPair<time_t, size_t>(time(NULL), length));

    return true;
}

void AntiDos::addKick(const QString &ip)
{
    QList<time_t> &l = kicksPerIp[ip];

    int i = 0;

    /* Removing kicks older than 15 minutes */
    while (i < l.size()) {
        if (time(NULL)-l[i] > 900) {
            i++;
        }  else {
            break;
        }
    }

    l.erase(l.begin(), l.begin()+i);

    l.push_back(time(NULL));

    if (l.size() >= ban_after_x_kicks && on) {
        emit ban(ip);
    }
}

void AntiDos::clearData()
{
    // Clears the history every 24 hours to avoid memory consumption
    loginsPerIp.clear();
    kicksPerIp.clear();
}

int AntiDos::connections(const QString &ip)
{
    return connectionsPerIp.value(ip);
}

QString AntiDos::dump() const
{
    return QString("Antidos\n\tConnections Per IP> %1\n\tLogins per IP> %2\n\tTransfers Per Id> %3\n\tSize of Transfers> %4\n\tKicks per IP> %5\n").arg(connectionsPerIp.count()).arg(
                loginsPerIp.count()).arg(transfersPerId.count()).arg(sizeOfTransfers.count()).arg(kicksPerIp.count());
}
