#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QSettings>
#include <QPushButton>
#include <QDebug>
#include "antidoswindow.h"
#include "antidos.h"

AntiDosWindow::AntiDosWindow(QSettings &settings) : settings(settings)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QFormLayout *mylayout = new QFormLayout(this);

    QSpinBox *mppip = new QSpinBox();
    mppip->setRange(1,8);
    mppip->setValue(settings.value("AntiDOS/MaxPeoplePerIp").toInt());
    mylayout->addRow(tr("Max number of people connected with the same ip"), mppip);

    QSpinBox *mcpus = new QSpinBox();
    mcpus->setRange(15,150);
    mcpus->setValue(settings.value("AntiDOS/MaxCommandsPerUser").toInt());
    mylayout->addRow(tr("Max number of times someone is active within a minute"), mcpus);

    QSpinBox *mkbpus = new QSpinBox();
    mkbpus->setRange(5,150);
    mkbpus->setValue(settings.value("AntiDOS/MaxKBPerUser").toInt());
    mkbpus->setSuffix(" kB");
    mylayout->addRow(tr("Maximum upload from user per minute"), mkbpus);

    QSpinBox *mlpip = new QSpinBox();
    mlpip->setRange(2,30);
    mlpip->setValue(settings.value("AntiDOS/MaxConnectionRatePerIP").toInt());
    mylayout->addRow(tr("Max number of times the same ip attempts to log in per minute"), mlpip);

    QSpinBox *baxk = new QSpinBox();
    baxk->setRange(1,30);
    baxk->setValue(settings.value("AntiDOS/NumberOfInfractionsBeforeBan").toInt());
    mylayout->addRow(tr("Bans after X antidos kicks per 15 minutes"), baxk);

    trusted_ips = new QLineEdit();
    trusted_ips->setText(settings.value("AntiDOS/TrustedIps").toString());
    mylayout->addRow(tr("Trusted IPs (separated by comma)"),trusted_ips);

    notificationsChannel = new QLineEdit(settings.value("AntiDOS/NotificationsChannel").toString());
    mylayout->addRow(tr("Channel in which to display overactive messages: "), notificationsChannel);

    QCheckBox *aDosOn = new QCheckBox(tr("Turn AntiDos ON"));
    aDosOn->setChecked(!settings.value("AntiDOS/Disabled").toBool());
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

    obj->trusted_ips = trusted_ips->text().split(QRegExp("\\s*,\\s*"));
    obj->max_people_per_ip = max_people_per_ip->value();
    obj->max_commands_per_user = max_commands_per_user->value();
    obj->max_kb_per_user = max_kb_per_user->value();
    obj->max_login_per_ip = max_login_per_ip->value();
    obj->ban_after_x_kicks = ban_after_x_kicks->value();
    obj->on = aDosOn->isChecked();
    obj->notificationsChannel = notificationsChannel->text();

    settings.setValue("AntiDOS/MaxPeoplePerIp", obj->max_people_per_ip);
    settings.setValue("AntiDOS/MaxCommandsPerUser", obj->max_commands_per_user);
    settings.setValue("AntiDOS/MaxKBPerUser", obj->max_kb_per_user);
    settings.setValue("AntiDOS/MaxConnectionRatePerIP", obj->max_login_per_ip);
    settings.setValue("AntiDOS/NumberOfInfractionsBeforeBan", obj->ban_after_x_kicks);
    settings.setValue("AntiDOS/TrustedIps", obj->trusted_ips.join(","));
    settings.setValue("AntiDOS/Disabled", !obj->on);
    settings.setValue("AntiDOS/NotificationsChannel", notificationsChannel->text());

    close();
}
