#include "serverconfig.h"
#include "../Utilities/otherwidgets.h"

ServerWindow::ServerWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QFormLayout *l = new QFormLayout(this);

    QSettings settings;
    serverName = new QLineEdit(settings.value("server_name").toString());

    serverName->setValidator(new QNickValidator(serverName));

    l->addRow("Server Name: ", serverName);

    serverDesc = new QTextEdit(settings.value("server_description").toString());

    l->addRow("Server Description: ", serverDesc);

    serverPlayerMax = new QSpinBox();
    serverPlayerMax->setMinimum(2);
    serverPlayerMax->setValue(settings.value("server_maxplayers").toInt());

    l->addRow("Max Players: ", serverPlayerMax);

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
    settings.setValue("server_name", serverName->text());
    settings.setValue("server_description", serverDesc->toPlainText());
    settings.setValue("server_maxplayers", serverPlayerMax->text());
    emit descChanged(serverDesc->toPlainText());
    emit nameChanged(serverName->text());
    emit maxChanged(serverPlayerMax->value());
    close();
}
