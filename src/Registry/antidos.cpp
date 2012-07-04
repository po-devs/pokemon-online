#include "antidos.h"

AntiDos::AntiDos() {

}

void AntiDos::init() {
    QSettings settings;
    /* initializing the default init values if not there */
    if (settings.value("AntiDOS/MaxPeoplePerIp").isNull()) {
        settings.setValue("AntiDOS/MaxPeoplePerIp", 5);
    }
    if (settings.value("AntiDOS/MaxCommandsPerUser").isNull()) {
        settings.setValue("AntiDOS/MaxCommandsPerUser", 200);
    }
    if (settings.value("AntiDOS/MaxKBPerUser").isNull()) {
        settings.setValue("AntiDOS/MaxKBPerUser", 40);
    }
    if (settings.value("AntiDOS/MaxConnectionRatePerIP").isNull()) {
        settings.setValue("AntiDOS/MaxConnectionRatePerIP", 20);
    }
    if (settings.value("AntiDOS/NumberOfInfractionsBeforeBan").isNull()) {
        settings.setValue("AntiDOS/NumberOfInfractionsBeforeBan", 10);
    }

    max_people_per_ip = settings.value("AntiDOS/MaxPeoplePerIp").toInt();
    max_commands_per_user = settings.value("AntiDOS/MaxCommandsPerUser").toInt();
    max_kb_per_user = settings.value("AntiDOS/MaxKBPerUser").toInt();
    max_login_per_ip = settings.value("AntiDOS/MaxConnectionRatePerIP").toInt();
    ban_after_x_kicks = settings.value("AntiDOS/NumberOfInfractionsBeforeBan").toInt();
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

        if (l.size() >= max_login_per_ip) {
            return false;
        }
    }

    if (connectionsPerIp.value(ip) >= max_people_per_ip) {
        return false;
    }


    /* Registering the connection */
    loginsPerIp[ip].push_back(time(NULL));
    connectionsPerIp[ip]++;

    return true;
}

void AntiDos::disconnect(const QString &ip, int id)
{
    connectionsPerIp[ip]--;
    transfersPerId.remove(id);
    sizeOfTransfers.remove(id);
}

bool AntiDos::transferBegin(int id, int length, const QString &ip)
{
    if (transfersPerId.contains(id)) {
        QList< QPair<time_t, size_t> > &l = transfersPerId[id];
        size_t &len = sizeOfTransfers[id];

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

        l.erase(l.begin(), l.begin()+i);

        if (l.size() >= max_commands_per_user) {
            emit kick(id);
            addKick(ip);
            return false;
        }

        if (len + length > size_t(max_kb_per_user)*1024) {
            emit kick(id);
            addKick(ip);
            return false;
        }
    } else if (length > max_kb_per_user*1024) {
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

    if (l.size() >= ban_after_x_kicks) {
        emit ban(ip);
    }
}

AntiDos * AntiDos::instance = new AntiDos();

