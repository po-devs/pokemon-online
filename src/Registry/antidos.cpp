#include "antidos.h"

AntiDos::AntiDos() {

}

void AntiDos::init() {
    QSettings settings;
    /* initializing the default init values if not there */
    if (settings.value("max_people_per_ip").isNull()) {
        settings.setValue("max_people_per_ip", 5);
    }
    if (settings.value("max_commands_per_user").isNull()) {
        settings.setValue("max_commands_per_user", 200);
    }
    if (settings.value("max_kbyte_per_user").isNull()) {
        settings.setValue("max_kbyte_per_user", 40);
    }
    if (settings.value("max_login_per_ip").isNull()) {
        settings.setValue("max_login_per_ip", 20);
    }
    if (settings.value("ban_after_X_kicks").isNull()) {
        settings.setValue("ban_after_X_kicks", 10);
    }

    max_people_per_ip = settings.value("max_people_per_ip").toInt();
    max_commands_per_user = settings.value("max_commands_per_user").toInt();
    max_kb_per_user = settings.value("max_kbyte_per_user").toInt();
    max_login_per_ip = settings.value("max_login_per_ip").toInt();
    ban_after_x_kicks = settings.value("ban_after_X_kicks").toInt();
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

