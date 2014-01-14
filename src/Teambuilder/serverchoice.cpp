#include "serverchoice.h"
#include "ui_serverchoice.h"
#include "analyze.h"
#include <Utilities/functions.h>
#include <PokemonInfo/networkstructs.h>
#include "serverchoicemodel.h"
#include "loadwindow.h"
#include "mainwindow.h"
#include <Utilities/otherwidgets.h>

ServerChoice::ServerChoice(TeamHolder* team) :
    ui(new Ui::ServerChoice), wasConnected(false), team(team)
{
    ui->setupUi(this);
    ui->announcement->hide();

    ServerChoiceModel *model = new ServerChoiceModel();
    model->setParent(ui->serverList);
    filter = new QSortFilterProxyModel(ui->serverList);
    filter->setSourceModel(model);
    filter->setSortRole(ServerChoiceModel::SortRole);
    ui->serverList->setModel(filter);

    connect(ui->description, SIGNAL(anchorClicked(QUrl)), SLOT(anchorClicked(QUrl)));

    QSettings settings;

    registry_connection = new Analyzer(true);
    connect(registry_connection, SIGNAL(connected()), SLOT(connected()));

    registry_connection->connectTo(
        settings.value("ServerChoice/RegistryServer", "registry.pokemon-online.eu").toString(),
        settings.value("ServerChoice/RegistryPort", 5090).toUInt()
    );
    registry_connection->setParent(this);

    ui->switchPort->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));

    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), ui->announcement, SLOT(setText(QString)));
    connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), ui->announcement, SLOT(show()));

    connect(registry_connection, SIGNAL(serverReceived(ServerInfo)), model, SLOT(addServer(ServerInfo)));
    connect(this, SIGNAL(clearList()), model, SLOT(clear()));
    connect(registry_connection, SIGNAL(serverReceived(ServerInfo)), SLOT(serverAdded()));

    //TO-DO: Make  the item 0 un-resizable and unselectable - Latios

    ui->serverList->setColumnWidth(0, settings.value("ServerChoice/PasswordProtectedWidth", 26).toInt());
    ui->serverList->setColumnWidth(1, settings.value("ServerChoice/ServerNameWidth", 152).toInt());
    if (settings.contains("ServerChoice/PlayersInfoWidth")) {
        ui->serverList->setColumnWidth(2, settings.value("ServerChoice/PlayersInfoWidth").toInt());
    }
    ui->serverList->horizontalHeader()->setStretchLastSection(true);

    connect(ui->serverList, SIGNAL(activated(QModelIndex)), SLOT(regServerChosen(QModelIndex)));
    connect(ui->serverList, SIGNAL(currentCellChanged(QModelIndex)), SLOT(showDetails(QModelIndex)));

    ui->nameEdit->setText(team->name());
    ui->advServerEdit->addItem(settings.value("ServerChoice/DefaultServer").toString());

    connect(ui->nameEdit, SIGNAL(returnPressed()), SLOT(advServerChosen()));
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

    connect(ui->teambuilder, SIGNAL(clicked()), SIGNAL(teambuilder()));
    connect(ui->advancedConnection, SIGNAL(clicked()), SLOT(advServerChosen()));

    QTimer *t = new QTimer(this);
    t->singleShot(5000, this, SLOT(timeout()));

#if QT_VERSION >= QT_VERSION_CHECK(4,8,0)
    ui->serverList->sortByColumn(ServerChoiceModel::Players, Qt::SortOrder(filter->headerData(ServerChoiceModel::Players, Qt::Horizontal, Qt::InitialSortOrderRole).toInt()));
#else
    ui->serverList->sortByColumn(ServerChoiceModel::Players, Qt::DescendingOrder);
#endif
}

ServerChoice::~ServerChoice()
{
    saveSettings();
    delete ui;
}

bool ServerChoice::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        e->accept();
    }

    return QFrame::event(e);
}

QMenuBar * ServerChoice::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();

    //TODO : Add menu allowing to change port / registry IP / ??

    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Load team"), this, SLOT(loadTeam()), tr("Ctrl+L", "Load team"));
    fileMenu->addAction(tr("New &tab"), w, SLOT(openNewTab()), tr("Ctrl+T", "New tab"));
    fileMenu->addAction(tr("Close tab"), w, SLOT(closeTab()), tr("Ctrl+W", "Close tab"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"),w,SLOT(quit()),tr("Ctrl+Q", "Quit"));

    w->addThemeMenu(menuBar);
    w->addStyleMenu(menuBar);
    w->addLanguageMenu(menuBar);

    QMenu *helpMenu = menuBar->addMenu(tr("&About"));
    helpMenu->addAction(tr("&Credits"), w, SLOT(launchCredits()));

    return menuBar;
}

void ServerChoice::loadTeam()
{
    LoadWindow *w = new LoadWindow(this, QStringList(), team->name());
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->show();

    connect(w, SIGNAL(teamLoaded(TeamHolder)), SLOT(loadAll(TeamHolder)));
}

void ServerChoice::loadAll(const TeamHolder &t)
{
    *team = t;
    ui->nameEdit->setText(t.name());
}

void ServerChoice::serverAdded()
{
    ui->serverList->sortByColumn(filter->sortColumn(), filter->sortOrder());
}

void ServerChoice::anchorClicked(const QUrl &url)
{
    if (wasConnected) {
        return;
    }
    if (url.scheme() == "po") {
        if (url.path() == "change-port") {
            on_switchPort_clicked();
        }
    }
}

void ServerChoice::on_switchPort_clicked()
{
    emit clearList();

    ui->serverList->model()->removeRows(0, ui->serverList->model()->rowCount());

    ui->description->setText(tr("Connecting to registry...")+"\n");

    QSettings settings;
    QString host =settings.value("ServerChoice/RegistryServer", "registry.pokemon-online.eu").toString();
    int port = settings.value("ServerChoice/RegistryPort", 5090).toUInt();
    int newport = port == 8080 ? 5090 : 8080;

    registry_connection->connectTo(host, newport);

    settings.setValue("ServerChoice/RegistryPort", newport);
}

void ServerChoice::connected()
{
    wasConnected = true;
    ui->description->setText(tr("Connected to the registry!"));
}

void ServerChoice::regServerChosen(const QModelIndex &i)
{
    if (!i.isValid()) {
        return;
    }

    QString ip = ui->serverList->model()->index(i.row(), ServerChoiceModel::IP).data().toString();
    QString name = ui->serverList->model()->index(i.row(), ServerChoiceModel::Name).data().toString();

    ui->advServerEdit->setItemText(ui->advServerEdit->currentIndex(), name + " - " + ip);

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

void ServerChoice::showDetails(const QModelIndex &i)
{
    if (!i.isValid()) {
        return;
    }
    ui->description->clear();
    ui->description->insertHtml(i.data(ServerChoiceModel::DescRole).toString());

    QString ip = ui->serverList->model()->index(i.row(), ServerChoiceModel::IP).data().toString();
    QString name = ui->serverList->model()->index(i.row(), ServerChoiceModel::Name).data().toString();

    ui->advServerEdit->setItemText(ui->advServerEdit->currentIndex(), name + " - " + ip);
}

void ServerChoice::connectionError(int, const QString &mess)
{
    ui->serverList->setCurrentIndex(QModelIndex());
    ui->description->clear();
    ui->description->insertPlainText(tr("Disconnected from the registry: %1").arg(mess) + "\n");

    if (!wasConnected) {
        ui->description->insertHtml(tr("You can try a different connection by <a href='po:change-port'>changing ports</a>."));
    }
}

void ServerChoice::timeout()
{
    if (wasConnected) {
        return;
    }

    ui->description->insertHtml(tr("Connection is taking longer than expected... You can try a <a href='po:change-port'>different connection</a>."));
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

    // Create default profile if none exist
    bool isNewProfile = settings.value("Profile/Current") == settings.value("Profile/Path");
    if (isNewProfile) {
        QNickValidator validator(0);
        if (validator.validate(ui->nameEdit->text()) == QValidator::Acceptable) {
            team->profile().name() = ui->nameEdit->text();
            QString path = settings.value("Profile/Path").toString()
                    + "/" + QUrl::toPercentEncoding(team->profile().name()) + ".xml";
            team->profile().saveToFile(path);
            settings.setValue("Profile/Current", team->profile().name());
        }
    }

    writeSettings(this);
}
