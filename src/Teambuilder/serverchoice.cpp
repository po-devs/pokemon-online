#include "serverchoice.h"
#include "../Utilities/functions.h"
#include "analyze.h"
#include "poketextedit.h"

ServerChoice::ServerChoice(const QString &nick)
{
    QSettings settings;

    registry_connection = new Analyzer(true);
    registry_connection->connectTo(
            settings.value("registry_server", "pokemon-online.dynalias.net").toString(),
            settings.value("registry_port", 5081).toUInt()
    );
    registry_connection->setParent(this);

    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    connect(registry_connection, SIGNAL(serverReceived(QString, QString, quint16,QString,quint16,quint16)), SLOT(addServer(QString, QString, quint16, QString,quint16,quint16)));

    QVBoxLayout *l = new QVBoxLayout(this);
    mylist = new QCompactTable(0,3);

    QStringList horHeaders;
    horHeaders << tr("Server Name") << tr("Players / Max") << tr("Advanced connection");
    mylist->setHorizontalHeaderLabels(horHeaders);
    mylist->setSelectionBehavior(QAbstractItemView::SelectRows);
    mylist->setSelectionMode(QAbstractItemView::SingleSelection);
    mylist->setShowGrid(false);
    mylist->verticalHeader()->hide();
    mylist->horizontalHeader()->resizeSection(0, 150);
    mylist->horizontalHeader()->setStretchLastSection(true);
    mylist->setMinimumHeight(200);

    connect(mylist, SIGNAL(cellActivated(int,int)), SLOT(regServerChosen(int)));
    connect(mylist, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(showDetails(int)));

    l->addWidget(mylist, 100);

    myDesc = new PokeTextEdit();
    myDesc->setOpenExternalLinks(true);
    myDesc->setFixedHeight(100);
    l->addWidget(new QEntitled("Server Description", myDesc));

    myName = new QLineEdit(nick);
    l->addWidget(new QEntitled("Trainer Name", myName));

    myAdvServer = new QLineEdit(settings.value("default_server").toString());
    connect(myAdvServer, SIGNAL(returnPressed()), SLOT(advServerChosen()));

    l->addWidget(new QEntitled("&Advanced Connection", myAdvServer));

    QHBoxLayout *hl= new QHBoxLayout();
    l->addLayout(hl);

    QPushButton *cancel = new QPushButton("&Go Back");
    QPushButton *ok = new QPushButton("Advanced &Connection");
    QPushButton *localhost = new QPushButton("Connect to own server");

    connect(cancel, SIGNAL(clicked()), SIGNAL(rejected()));
    connect(ok, SIGNAL(clicked()), SLOT(advServerChosen()));
    connect(localhost, SIGNAL(clicked()), SLOT(connectToLocalhost()));

    hl->addWidget(cancel);
    hl->addWidget(ok);
    hl->addWidget(localhost);
}

ServerChoice::~ServerChoice()
{
    writeSettings(this);
}

void ServerChoice::regServerChosen(int row)
{
    QString ip = mylist->item(row, 2)->text();

    QSettings settings;
    settings.setValue("default_server", ip);
    if(ip.contains(":")){
        quint16 port = ip.section(":",1,1).toInt(); //Gets port from IP:PORT
        QString fIp = ip.section(":",0,0);  //Gets IP from IP:PORT
        emit serverChosen(fIp,port, myName->text());
    }
    else
        emit serverChosen(ip,5080, myName->text());
}

void ServerChoice::advServerChosen()
{
    QString ip = myAdvServer->text().trimmed();

    QSettings settings;
    settings.setValue("default_server", ip);
    if(ip.contains(":")){
        quint16 port = ip.section(":",1,1).toInt(); //Gets port from IP:PORT
        QString fIp = ip.section(":",0,0);  //Gets IP from IP:PORT
        emit serverChosen(fIp,port, myName->text());
    }
    else
        emit serverChosen(ip,5080, myName->text());

}
void ServerChoice::connectToLocalhost()
{
    emit serverChosen("localhost", 5080, myName->text());
}

void ServerChoice::addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, quint16 max, quint16 port)
{
    mylist->setSortingEnabled(false);

    QString playerStr;
    if(max == 0)
        playerStr = QString::number(num).rightJustified(3);
    else
        playerStr = QString::number(num).rightJustified(3) + " / " + QString::number(max);
    int row = mylist->rowCount();
    mylist->setRowCount(row+1);

    QTableWidgetItem *witem;

    witem = new QTableWidgetItem(name);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 0, witem);

    witem = new QTableWidgetItem(playerStr);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 1, witem);

    witem = new QTableWidgetItem(ip + ":" + QString::number(port == 0 ? 5080 : port));
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 2, witem);

    descriptionsPerIp.insert(ip + ":" + QString::number(port == 0 ? 5080 : port), desc);
    /*This needed to be changed because the showDescription function was looking for a ip and port,
      while only the IP was in the list, and in the end, the description wouldn't be displayed. */

    mylist->setSortingEnabled(true);
    mylist->sortByColumn(1);

    if (mylist->currentRow() != -1)
        showDetails(mylist->currentRow());
}

void ServerChoice::showDetails(int row)
{
    if (row < 0)
        return;
    myDesc->clear();
    myDesc->insertHtml(descriptionsPerIp[mylist->item(row,2)->text()]);

    QString ip = mylist->item(row, 2)->text();
    myAdvServer->setText(ip);

}

void ServerChoice::connectionError(int, const QString &mess)
{
    mylist->setCurrentCell(-1,-1);
    myDesc->clear();
    myDesc->insertPlainText(tr("Disconnected from the registry: %1").arg(mess));
}
