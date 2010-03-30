#include "serverchoice.h"
#include "../Utilities/otherwidgets.h"
#include "analyze.h"

ServerChoice::ServerChoice()
{
    resize(500,450);

    registry_connection = new Analyzer(true);
    registry_connection->connectTo("pokeymon.zapto.org", 5081);
    registry_connection->setParent(this);

    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    connect(registry_connection, SIGNAL(serverReceived(QString, QString, quint16,QString,quint16)), SLOT(addServer(QString, QString, quint16, QString,quint16)));

    QVBoxLayout *l = new QVBoxLayout(this);
    mylist = new QCompactTable(0,3);

    QStringList horHeaders;
    horHeaders << tr("Server Name") << tr("Players/Max Pla") << tr("Advanced connection");
    mylist->setHorizontalHeaderLabels(horHeaders);
    mylist->setSelectionBehavior(QAbstractItemView::SelectRows);
    mylist->setSelectionMode(QAbstractItemView::SingleSelection);
    mylist->setShowGrid(false);
    mylist->verticalHeader()->hide();
    mylist->horizontalHeader()->setStretchLastSection(true);
    mylist->setMinimumHeight(200);

    connect(mylist, SIGNAL(cellActivated(int,int)), SLOT(regServerChosen(int)));
    connect(mylist, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(showDescription(int)));

    l->addWidget(mylist, 100);

    myDesc = new QTextEdit();
    myDesc->setReadOnly(true);
    myDesc->setFixedHeight(100);
    l->addWidget(new QEntitled("Server Description", myDesc));

    QSettings settings;
    myAdvServer = new QLineEdit(settings.value("default_server").toString());
    connect(myAdvServer, SIGNAL(returnPressed()), SLOT(advServerChosen()));

    l->addWidget(new QEntitled("&Advanced Connection", myAdvServer));

    QHBoxLayout *hl= new QHBoxLayout();
    l->addLayout(hl);

    QPushButton *cancel = new QPushButton("&Go Back");
    QPushButton *ok = new QPushButton("Advanced &Connection");

    connect(cancel, SIGNAL(clicked()), SIGNAL(rejected()));
    connect(ok, SIGNAL(clicked()), SLOT(advServerChosen()));

    hl->addWidget(cancel);
    hl->addWidget(ok);
}

void ServerChoice::regServerChosen(int row)
{
    QString ip = mylist->item(row, 2)->text();

    QSettings settings;
    settings.setValue("default_server", ip);
    emit serverChosen(ip);
}

void ServerChoice::advServerChosen()
{
    QString ip = myAdvServer->text().trimmed();

    QSettings settings;
    settings.setValue("default_server", ip);
    emit serverChosen(ip);
}

void ServerChoice::addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, const quint16 max)
{
    QString playerStr;
    if(max == 0)
        playerStr = QString::number(num);
    else
        playerStr = QString::number(num) + "/" + max;
    int row = mylist->rowCount();
    mylist->setRowCount(row+1);

    QTableWidgetItem *witem;

    witem = new QTableWidgetItem(name);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 0, witem);

    witem = new QTableWidgetItem(playerStr.rightJustified(3));
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 1, witem);

    witem = new QTableWidgetItem(ip);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 2, witem);

    mylist->resizeColumnsToContents();
    mylist->resizeRowsToContents();
    mylist->horizontalHeader()->setStretchLastSection(true);

    descriptionsPerIp.insert(ip, desc);

    if (mylist->currentRow() != -1)
        showDescription(mylist->currentRow());
}

void ServerChoice::showDescription(int row)
{
    if (row < 0)
        return;
    myDesc->clear();
    myDesc->insertPlainText(descriptionsPerIp[mylist->item(row,2)->text()]);
}

void ServerChoice::connectionError(int, const QString &mess)
{
    mylist->setCurrentCell(-1,-1);
    myDesc->clear();
    myDesc->insertPlainText(tr("Disconnected from the registry: %1").arg(mess));
}
