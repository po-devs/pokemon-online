#include "serverconfig.h"
#include "server.h"
#include "../Utilities/otherwidgets.h"

ServerWindow::ServerWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QFormLayout *l = new QFormLayout(this);

    QSettings settings("config", QSettings::IniFormat);

    serverPrivate = new QComboBox;
    serverPrivate->addItem("Public");
    serverPrivate->addItem("Private");
    serverPrivate->setCurrentIndex(settings.value("server_private").toInt());
    l->addRow("Public/Private: ", serverPrivate);

    serverName = new QLineEdit(settings.value("server_name").toString());
    mainChan = new QLineEdit(settings.value("mainchanname").toString());

    serverName->setValidator(new QNickValidator(serverName));
    mainChan->setValidator(new QNickValidator(mainChan));

    l->addRow("Server Name: ", serverName);
    l->addRow("Main Channel: ", mainChan);

    serverDesc = new QPlainTextEdit(settings.value("server_description").toString());

    l->addRow("Server Description: ", serverDesc);

    serverAnnouncement = new QPlainTextEdit(settings.value("server_announcement").toString());

    l->addRow("Announcement: ", serverAnnouncement);

    serverPlayerMax = new QSpinBox();
    serverPlayerMax->setRange(0,5000);
    serverPlayerMax->setSpecialValueText(tr("unlimited"));
    serverPlayerMax->setSingleStep(10);
    serverPlayerMax->setValue(settings.value("server_maxplayers").toInt());

    l->addRow("Max Players: ", serverPlayerMax);

    serverPort = new QSpinBox();
    serverPort->setRange(0,65535);
    if(settings.value("server_port").toInt() == 0)
        serverPort->setValue(5080);
    else
        serverPort->setValue(settings.value("server_port").toInt());

    l->addRow("Port(requires restart): ", serverPort);
    QPushButton *ok, *cancel;

    l->addRow("Extended Logging: ", saveLogs = new QCheckBox("Show all player events and all logging"));
    saveLogs->setChecked(settings.value("show_log_messages").toBool());

    l->addRow("File Logging for Channels: ", channelFileLog = new QCheckBox("Save channel messages to daily rotating log files"));
    channelFileLog->setChecked(settings.value("logs_channel_files").toBool());

    l->addRow("File Logging for Battle: ", battleFileLog = new QCheckBox("Save battle logs to files"));
    battleFileLog->setChecked(settings.value("logs_battle_files").toBool());

    l->addRow("Low Latency: ", lowLatency = new QCheckBox("Sacrifices bandwith for latency (look up Nagle's algorithm)"));
    lowLatency->setChecked(settings.value("low_TCP_delay").toBool());
    
    l->addRow("Safe scripts: ", safeScripts = new QCheckBox("Restricts some script functions to improve security."));
    safeScripts->setChecked(settings.value("safe_scripts").toBool());

    l->addRow("Proxy Servers: ", proxyServers = new QLineEdit(settings.value("proxyservers").toString()));

    ok = new QPushButton("&Apply");
    cancel = new QPushButton("&Cancel");

    l->addRow(ok, cancel);

    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ok, SIGNAL(clicked()), SLOT(apply()));
}

void ServerWindow::apply()
{
    QSettings settings("config", QSettings::IniFormat);
    settings.setValue("server_private", serverPrivate->currentIndex());
    settings.setValue("server_name", serverName->text());
    settings.setValue("server_description", serverDesc->toPlainText());
    settings.setValue("server_maxplayers", serverPlayerMax->text());
    settings.setValue("server_port", serverPort->text());
    settings.setValue("server_announcement", serverAnnouncement->toPlainText());
    settings.setValue("show_log_messages", saveLogs->isChecked());
    settings.setValue("logs_channel_files", channelFileLog->isChecked());
    settings.setValue("logs_battle_files", channelFileLog->isChecked());
    settings.setValue("mainchanname", mainChan->text());
    settings.setValue("low_TCP_delay", lowLatency->isChecked());
    settings.setValue("safe_scripts", safeScripts->isChecked());
    settings.setValue("proxyservers", proxyServers->text());

    emit descChanged(serverDesc->toPlainText());
    emit nameChanged(serverName->text());
    emit maxChanged(serverPlayerMax->value());
    emit announcementChanged(serverAnnouncement->toPlainText());
    emit privacyChanged(serverPrivate->currentIndex());
    emit logSavingChanged(saveLogs->isChecked());
    emit useChannelFileLogChanged(channelFileLog->isChecked());
    emit useBattleFileLogChanged(battleFileLog->isChecked());
    if (mainChan->text().length() > 0)
        emit mainChanChanged(mainChan->text());
    emit latencyChanged(lowLatency->isChecked());
    emit safeScriptsChanged(safeScripts->isChecked());
    emit proxyServersChanged(proxyServers->text());

    close();
}
