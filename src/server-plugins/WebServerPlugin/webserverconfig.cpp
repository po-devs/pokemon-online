#include "webserverplugin.h"
#include "webserverconfig.h"
#include "ui_webserverconfig.h"

WebServerConfig::WebServerConfig(WebServerPlugin *parent) :
    QDialog(),
    ui(new Ui::WebServerConfig),
    master(parent)
{
    ui->setupUi(this);

    ui->port->setText(QString::number(master->port));
    ui->password->setText(master->pass);

    connect(this, SIGNAL(accepted()), SLOT(close()));
    connect(this, SIGNAL(rejected()), SLOT(close()));
}

WebServerConfig::~WebServerConfig()
{
    delete ui;
}

void WebServerConfig::accept()
{
    QSettings settings("config", QSettings::IniFormat);
    settings.setValue("WebServer/Port", ui->port->text());
    settings.setValue("WebServer/Password", ui->password->text());
    master->port = ui->port->text().toInt();
    master->pass = ui->password->text();
}
