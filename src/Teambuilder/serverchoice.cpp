#include "serverchoice.h"
#include "ui_serverchoice.h"
#include "analyze.h"
#include "../Utilities/functions.h"
#include "theme.h"

ServerChoice::ServerChoice(const QString &nick) :
    ui(new Ui::ServerChoice)
{
    ui->setupUi(this);

    QSettings settings;

    registry_connection = new Analyzer(true);
    registry_connection->connectTo(
            settings.value("ServerChoice/registry_server", "pokemon-online-registry.dynalias.net").toString(),
            settings.value("ServerChoice/registry_port", 8080).toUInt()
    );
    registry_connection->setParent(this);

    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), ui->announcement, SLOT(setText(QString)));

    connect(registry_connection, SIGNAL(serverReceived(QString, QString, quint16,QString,quint16,quint16, bool)), SLOT(addServer(QString, QString, quint16, QString,quint16,quint16, bool)));

    //TO-DO: Make  the item 0 un-resizable and unselectable - Latios

    ui->serverList->setColumnWidth(0, settings.value("ServerChoice/PasswordProtectedWidth", 26).toInt());
    ui->serverList->setColumnWidth(1, settings.value("ServerChoice/ServerNameWidth", 152).toInt());
    if (settings.contains("ServerChoice/PlayersInfoWidth")) {
        ui->serverList->setColumnWidth(2, settings.value("ServerChoice/PlayersInfoWidth").toInt());
    }
    ui->serverList->horizontalHeader()->setStretchLastSection(true);
    ui->serverList->horizontalHeaderItem(0)->setIcon(Theme::unlockedLockedRegistry());

    connect(ui->serverList, SIGNAL(cellActivated(int,int)), SLOT(regServerChosen(int)));
    connect(ui->serverList, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(showDetails(int)));

    ui->nameEdit->setText(nick);
    ui->advServerEdit->addItem(settings.value("ServerChoice/DefaultServer").toString());
    connect(ui->advServerEdit->lineEdit(), SIGNAL(returnPressed()), SLOT(advServerChosen()));

    QCompleter *completer = new QCompleter(ui->advServerEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    QStringList res = settings.value("ServerChoice/SavedServers").toStringList();

    foreach (QString r, res) {
        if (r.contains("-")) {
            savedServers.push_back(QStringList() << r.section("-", -1).trimmed() << r.section("-", 0, -2).trimmed());
        } else {
            savedServers.push_back(QStringList() << r << "");
        }
    }

    QStringListModel *m = new QStringListModel(res, completer);

    completer->setModel(m);
    ui->advServerEdit->setCompleter(completer);
    ui->advServerEdit->setModel(m);

    connect(ui->goBack, SIGNAL(clicked()), SIGNAL(rejected()));
    connect(ui->advancedConnection, SIGNAL(clicked()), SLOT(advServerChosen()));
}

ServerChoice::~ServerChoice()
{
    saveSettings();
    writeSettings(this);
    delete ui;
}

void ServerChoice::regServerChosen(int row)
{
    QString ip = ui->serverList->item(row, 3)->text();
    QString name = ui->serverList->item(row, 1)->text();

    QSettings settings;
    settings.setValue("ServerChoice/DefaultServer", name  + " - " + ip);
    if(ip.contains(":")){
        quint16 port = ip.section(":",1,1).toInt();
        QString fIp = ip.section(":",0,0);
        emit serverChosen(fIp,port, ui->nameEdit->text());
    } else {
        emit serverChosen(ip,5080, ui->nameEdit->text());
    }
    addSavedServer(ip, name);
}

void ServerChoice::advServerChosen()
{
    QString info = ui->advServerEdit->currentText();
    QString ip = info.section("-", -1).trimmed();
    QString name = info.contains("-") ? info.section("-", 0, -2).trimmed() : "";

    QSettings MySettings;
    MySettings.setValue("ServerChoice/DefaultServer", ui->advServerEdit->currentText());
    if(info.contains(":")) {
        quint16 port = ip.section(":",1,1).toInt();
        QString fIp = ip.section(":",0,0);
        emit serverChosen(fIp,port, ui->nameEdit->text());
    } else {
        emit serverChosen(info,5080, ui->nameEdit->text());
    }
    addSavedServer(ip, name);
}

void ServerChoice::addSavedServer(const QString &ip, const QString &name)
{
    if (name.length() == 0) {
        for (int i = 0; i < savedServers.length(); i++) {
            if (savedServers[i][0] == ip) {
                savedServers.push_front(savedServers.takeAt(i));
                return;
            }
        }
        savedServers.push_front(QStringList() << ip << name);
    } else {
        for (int i = 0; i < savedServers.length(); i++) {
            if (savedServers[i][0] == ip || savedServers[i][1] == name) {
                savedServers.removeAt(i);
                break;
            }
        }
        savedServers.push_front(QStringList() << ip << name);
    }
    if (savedServers.length() > 10) {
        savedServers.erase(savedServers.begin()+10, savedServers.end());
    }
}

void ServerChoice::addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, quint16 max, quint16 port, bool passwordProtected)
{
    ui->serverList->setSortingEnabled(false);

    QString playerStr;
    if(max == 0)
        playerStr = QString::number(num).rightJustified(5);
    else
        playerStr = QString::number(num).rightJustified(5) + " / " + QString::number(max);
    int row = ui->serverList->rowCount();
    ui->serverList->setRowCount(row+1);

    ui->serverList->setItem(row, 0, passwordProtected ? new QTableWidgetItem(Theme::lockedServer(), "") : new QTableWidgetItem(Theme::unlockedServer(), ""));
    ui->serverList->setItem(row, 1, new QTableWidgetItem(name));
    ui->serverList->setItem(row, 2, new QTableWidgetItem(playerStr));
    ui->serverList->setItem(row, 3, new QTableWidgetItem(ip + ":" + QString::number(port == 0 ? 5080 : port)));

    descriptionsPerIp.insert(ip + ":" + QString::number(port == 0 ? 5080 : port), desc);
    /*This needed to be changed because the showDescription function was looking for a ip and port,
      while only the IP was in the list, and in the end, the description wouldn't be displayed. */

    ui->serverList->setSortingEnabled(true);
    ui->serverList->sortByColumn(2);

    if (ui->serverList->currentRow() != -1)
        showDetails(ui->serverList->currentRow());
}

void ServerChoice::showDetails(int row)
{
    if (row < 0)
        return;
    ui->description->clear();
    ui->description->insertHtml(descriptionsPerIp[ui->serverList->item(row,3)->text()]);

    QString ip = ui->serverList->item(row, 3)->text();
    QString name = ui->serverList->item(row, 1)->text();

    ui->advServerEdit->setItemText(ui->advServerEdit->currentIndex(), name + " - " + ip);
}

void ServerChoice::connectionError(int, const QString &mess)
{
    ui->serverList->setCurrentCell(-1,-1);
    ui->description->clear();
    ui->description->insertPlainText(tr("Disconnected from the registry: %1").arg(mess));
}

void ServerChoice::saveSettings() {
    QSettings settings;
    settings.setValue("ServerChoice/PasswordProtectedWidth", ui->serverList->columnWidth(0));
    settings.setValue("ServerChoice/ServerNameWidth", ui->serverList->columnWidth(1));
    settings.setValue("ServerChoice/PlayersInfoWidth", ui->serverList->columnWidth(2));
    settings.setValue("ServerChoice/ServerIPWidth", ui->serverList->columnWidth(3));

    QStringList res;
    foreach (QStringList list, savedServers) {
        if (list[1].length() > 0) {
            res << QString("%1 - %2").arg(list[1], list[0]);
        } else {
            res << list[0];
        }
    }
    settings.setValue("ServerChoice/SavedServers", res);
}
