#include "antidos.h"

AntiDosWindow::AntiDosWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QFormLayout *mylayout = new QFormLayout(this);

    QSettings settings;

    QSpinBox *mppip = new QSpinBox();
    mppip->setRange(1,8);
    mppip->setValue(settings.value("max_people_per_ip").toInt());
    mylayout->addRow(tr("Max number of people connected with the same ip"), mppip);

    QSpinBox *mcpus = new QSpinBox();
    mcpus->setRange(15,150);
    mcpus->setValue(settings.value("max_commands_per_user").toInt());
    mylayout->addRow(tr("Max number of times someone is active within a minute"), mcpus);

    QSpinBox *mkbpus = new QSpinBox();
    mkbpus->setRange(5,150);
    mkbpus->setValue(settings.value("max_kbyte_per_user").toInt());
    mkbpus->setSuffix(" kB");
    mylayout->addRow(tr("Maximum upload from user per minute"), mkbpus);

    QSpinBox *mlpip = new QSpinBox();
    mlpip->setRange(2,30);
    mlpip->setValue(settings.value("max_login_per_ip").toInt());
    mylayout->addRow(tr("Max number of times the same ip attempts to log in per minute"), mlpip);

    QSpinBox *baxk = new QSpinBox();
    baxk->setRange(1,30);
    baxk->setValue(settings.value("ban_after_X_kicks").toInt());
    mylayout->addRow(tr("Bans after X antidos kicks per 15 minutes"), baxk);

    QCheckBox *aDosOn = new QCheckBox(tr("Turn AntiDos ON"));
    aDosOn->setChecked(!settings.value("antidos_off").toBool());
    mylayout->addWidget(aDosOn);

    QPushButton *ok = new QPushButton("&Apply");
    QPushButton *cancel = new QPushButton("&Cancel");

    mylayout->addRow(ok, cancel);

    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ok, SIGNAL(clicked()), SLOT(apply()));

    max_people_per_ip = mppip;
    max_commands_per_user = mcpus;
    max_kb_per_user = mkbpus;
    max_login_per_ip = mlpip;
    ban_after_x_kicks = baxk;
    this->aDosOn = aDosOn;
}

void AntiDosWindow::apply()
{
    AntiDos *obj = AntiDos::obj();

    obj->max_people_per_ip = max_people_per_ip->value();
    obj->max_commands_per_user = max_commands_per_user->value();
    obj->max_kb_per_user = max_kb_per_user->value();
    obj->max_login_per_ip = max_login_per_ip->value();
    obj->ban_after_x_kicks = ban_after_x_kicks->value();
    obj->on = aDosOn->isChecked();

    QSettings settings;
    /* initializing the default init values if not there */

    settings.setValue("max_people_per_ip", obj->max_people_per_ip);
    settings.setValue("max_commands_per_user", obj->max_commands_per_user);
    settings.setValue("max_kbyte_per_user", obj->max_kb_per_user);
    settings.setValue("max_login_per_ip", obj->max_login_per_ip);
    settings.setValue("ban_after_X_kicks", obj->ban_after_x_kicks);
    settings.setValue("antidos_off", !obj->on);

    close();
}

AntiDos::AntiDos() {
    // Clears history every day, to save RAM.
    timer.start(24*3600, this);
}

void AntiDos::init() {
    QSettings settings;
    /* initializing the default init values if not there */
    if (settings.value("max_people_per_ip").isNull()) {
        settings.setValue("max_people_per_ip", 2);
    }
    if (settings.value("max_commands_per_user").isNull()) {
        settings.setValue("max_commands_per_user", 50);
    }
    if (settings.value("max_kbyte_per_user").isNull()) {
        settings.setValue("max_kbyte_per_user", 25);
    }
    if (settings.value("max_login_per_ip").isNull()) {
        settings.setValue("max_login_per_ip", 6);
    }
    if (settings.value("ban_after_X_kicks").isNull()) {
        settings.setValue("ban_after_X_kicks", 10);
    }
    if (settings.value("antidos_off").isNull()) {
        settings.setValue("antidos_off", false);
    }


    max_people_per_ip = settings.value("max_people_per_ip").toInt();
    max_commands_per_user = settings.value("max_commands_per_user").toInt();
    max_kb_per_user = settings.value("max_kbyte_per_user").toInt();
    max_login_per_ip = settings.value("max_login_per_ip").toInt();
    ban_after_x_kicks = settings.value("ban_after_X_kicks").toInt();
    on = !settings.value("antidos_off").toBool();
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

    return true;
}

void AntiDos::disconnect(const QString &ip, int id)
{
    connectionsPerIp[ip]--;
    transfersPerId.remove(id);
    sizeOfTransfers.remove(id);
    if (connectionsPerIp[ip]==0) {
        connectionsPerIp.remove(ip);
    }
}

int AntiDos::numberOfDiffIps()
{
    return connectionsPerIp.count();
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

        if (l.size() >= max_commands_per_user && on) {
            emit kick(id);
            addKick(ip);
            return false;
        }

        if (len + length > size_t(max_kb_per_user)*1024 && on) {
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

    if (l.size() >= ban_after_x_kicks && on) {
        emit ban(ip);
    }
}

void AntiDos::timerEvent(QTimerEvent *)
{
    // Clears the history every 24 hours to avoid memory consumption
    loginsPerIp.clear();
    kicksPerIp.clear();
}

AntiDos * AntiDos::instance = new AntiDos();
