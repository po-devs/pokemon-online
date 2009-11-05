#include "client.h"
#include "mainwindow.h"

#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t) : myteam(t), myrelay(this)
{
    setFixedSize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1);
    layout->addWidget(mychat = new QTextEdit(), 0, 1);
    layout->addWidget(myline = new QLineEdit(), 1, 1);
    layout->addWidget(myexit = new QPushButton(tr("Exit")), 2, 1);

    myplayers->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mychat->setReadOnly(true);

    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));

    initRelay();
}

QMenuBar * Client::createMenuBar(MainWindow *w)
{
    (void) w;
    return NULL;
}

void Client::initRelay()
{
    connect(&relay(), SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(&relay(), SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
}

void Client::errorFromNetwork(int errno, const QString &errorDesc)
{
    (void) errno;
    (void) errorDesc;
}

Analyzer &Client::relay()
{
    return myrelay;
}
