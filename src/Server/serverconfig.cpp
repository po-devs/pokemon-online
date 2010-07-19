#include "serverconfig.h"
#include "server.h"
#include "../Utilities/otherwidgets.h"

ServerWindow::ServerWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QFormLayout *l = new QFormLayout(this);

    QSettings settings;

    serverPrivate = new QComboBox;
    serverPrivate->addItem("Public");
    serverPrivate->addItem("Private");
    serverPrivate->setCurrentIndex(settings.value("server_private").toInt());
    l->addRow("Public/Private: ", serverPrivate);

    serverName = new QLineEdit(settings.value("server_name").toString());

    serverName->setValidator(new QNickValidator(serverName));

    l->addRow("Server Name: ", serverName);

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

    ok = new QPushButton("&Apply");
    cancel = new QPushButton("&Cancel");

    l->addRow(ok, cancel);

    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ok, SIGNAL(clicked()), SLOT(apply()));
}

void ServerWindow::apply()
{
    QSettings settings;
    settings.setValue("server_private", serverPrivate->currentIndex());
    settings.setValue("server_name", serverName->text());
    settings.setValue("server_description", serverDesc->toPlainText());
    settings.setValue("server_maxplayers", serverPlayerMax->text());
    settings.setValue("server_port", serverPort->text());
    settings.setValue("server_announcement", serverAnnouncement->toPlainText());
    emit descChanged(serverDesc->toPlainText());
    emit nameChanged(serverName->text());
    emit maxChanged(serverPlayerMax->value());
    emit announcementChanged(serverAnnouncement->toPlainText());
    emit privacyChanged(serverPrivate->currentIndex());
    close();
}
