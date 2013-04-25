#include <QFormLayout>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>

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
    serverPrivate->setCurrentIndex(settings.value("Server/Private").toInt());
    l->addRow("Public/Private: ", serverPrivate);

    serverName = new QLineEdit(settings.value("Server/Name").toString());
    mainChan = new QLineEdit(settings.value("Channels/MainChannel").toString());

    serverName->setValidator(new QNickValidator(serverName));
    mainChan->setValidator(new QNickValidator(mainChan));

    l->addRow("Server Name: ", serverName);
    l->addRow("Main Channel: ", mainChan);

    serverDesc = new QPlainTextEdit(settings.value("Server/Description").toString());

    l->addRow("Server Description: ", serverDesc);

    serverAnnouncement = new QPlainTextEdit(settings.value("Server/Announcement").toString());

    l->addRow("Announcement: ", serverAnnouncement);

    serverPlayerMax = new QSpinBox();
    serverPlayerMax->setRange(0,5000);
    serverPlayerMax->setSpecialValueText(tr("unlimited"));
    serverPlayerMax->setSingleStep(10);
    serverPlayerMax->setValue(settings.value("Server/MaxPlayers").toInt());

    l->addRow("Max Players: ", serverPlayerMax);

    serverPort = new QSpinBox();
    serverPort->setRange(0,65535);
    if(settings.value("Network/Ports").toInt() == 0)
        serverPort->setValue(5080);
    else
        serverPort->setValue(settings.value("Network/Ports").toInt());

    l->addRow("Port(requires restart): ", serverPort);
    QPushButton *ok, *cancel;

    l->addRow("Extended Logging: ", saveLogs = new QCheckBox("Show all player events and all logging"));
    saveLogs->setChecked(settings.value("GUI/ShowLogMessages").toBool());

    l->addRow("File Logging for Channels: ", channelFileLog = new QCheckBox("Save channel messages to daily rotating log files"));
    channelFileLog->setChecked(settings.value("Channels/LoggingEnabled").toBool());

    l->addRow("Delete inactive members in a period of (Days):", deleteInactive = new QSpinBox());
    deleteInactive->setRange(1, 728);
    if(settings.value("Players/InactiveThresholdInDays").isNull()) {
        deleteInactive->setValue(182);
    } else {
        deleteInactive->setValue(settings.value("Players/InactiveThresholdInDays").toInt());
    }

    l->addRow("Low Latency: ", lowLatency = new QCheckBox("Sacrifices bandwith for latency (look up Nagle's algorithm)"));
    lowLatency->setChecked(settings.value("Network/LowTCPDelay").toBool());
    
    l->addRow("Safe scripts: ", safeScripts = new QCheckBox("Restricts some script functions to improve security."));
    safeScripts->setChecked(settings.value("Scripts/SafeMode").toBool());
    
    l->addRow("Minimize to tray: ", minimizeToTray = new QCheckBox("Hide to tray when minimized/switch desktop."));
    minimizeToTray->setChecked(settings.value("GUI/MinimizeToTray").toBool());

    l->addRow("Show tray popup: ", trayPopup = new QCheckBox("Show tooltip when PO is minimized to tray."));
    trayPopup->setChecked(settings.value("GUI/ShowTrayPopup").toBool());

    l->addRow("Double Click tray icon", doubleClick = new QCheckBox("Double click to reopen when PO is minimized to tray."));
    doubleClick->setChecked(settings.value("GUI/DoubleClickIcon").toBool());

    l->addRow("Show Overactive Messages: ", showOveractive = new QCheckBox("Show Overactive Message when someone goes overactive"));
    showOveractive->setChecked(settings.value("AntiDOS/ShowOveractiveMessages").toBool());

    l->addRow("Proxy Servers: ", proxyServers = new QLineEdit(settings.value("Network/ProxyServers").toString()));

    l->addRow(usePassword = new QCheckBox("Require Password: "), serverPassword = new QLineEdit(settings.value("Server/Password").toString()));
    usePassword->setChecked(settings.value("Server/RequirePassword").toBool());

    ok = new QPushButton("&Apply");
    cancel = new QPushButton("&Cancel");

    l->addRow(ok, cancel);

    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ok, SIGNAL(clicked()), SLOT(apply()));
}

void ServerWindow::apply()
{
    if (usePassword->isChecked() && serverPassword->text().length() == 0) {
        QMessageBox msgBox;
        msgBox.setText("You need to set the server password if you require it.");
        msgBox.exec();
        return;
    }

    QSettings settings("config", QSettings::IniFormat);
    settings.setValue("Server/Private", serverPrivate->currentIndex());
    settings.setValue("Server/Name", serverName->text());
    settings.setValue("Server/Description", serverDesc->toPlainText());
    settings.setValue("Server/MaxPlayers", serverPlayerMax->text());
    settings.setValue("Network/Ports", serverPort->text());
    settings.setValue("Server/Announcement", serverAnnouncement->toPlainText());
    settings.setValue("GUI/ShowLogMessages", saveLogs->isChecked());
    settings.setValue("Channels/LoggingEnabled", channelFileLog->isChecked());
    settings.setValue("Players/InactiveThresholdInDays", deleteInactive->text());
    settings.setValue("Channels/MainChannel", mainChan->text());
    settings.setValue("Network/LowTCPDelay", lowLatency->isChecked());
    settings.setValue("Scripts/SafeMode", safeScripts->isChecked());
    settings.setValue("GUI/MinimizeToTray", minimizeToTray->isChecked());
    settings.setValue("GUI/ShowTrayPopup", trayPopup->isChecked());
    settings.setValue("GUI/DoubleClickIcon", doubleClick->isChecked());
    settings.setValue("AntiDOS/ShowOveractiveMessages", showOveractive->isChecked());
    settings.setValue("Network/ProxyServers", proxyServers->text());
    settings.setValue("Server/Password", serverPassword->text());
    settings.setValue("Server/RequirePassword", usePassword->isChecked());

    emit descChanged(serverDesc->toPlainText());
    emit nameChanged(serverName->text());
    emit maxChanged(serverPlayerMax->value());
    emit announcementChanged(serverAnnouncement->toPlainText());
    emit privacyChanged(serverPrivate->currentIndex());
    emit logSavingChanged(saveLogs->isChecked());
    emit useChannelFileLogChanged(channelFileLog->isChecked());
    emit mainChanChanged(mainChan->text());
    emit inactivePlayersDeleteDaysChanged(deleteInactive->value());
    emit latencyChanged(lowLatency->isChecked());
    emit safeScriptsChanged(safeScripts->isChecked());
    emit overactiveToggleChanged(showOveractive->isChecked());
    emit proxyServersChanged(proxyServers->text());
    emit serverPasswordChanged(serverPassword->text());
    emit usePasswordChanged(usePassword->isChecked());
    emit minimizeToTrayChanged(minimizeToTray->isChecked());
    emit showTrayPopupChanged(trayPopup->isChecked());
    emit clickConditionChanged(doubleClick->isChecked());

    close();
}
