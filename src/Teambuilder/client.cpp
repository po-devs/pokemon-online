#include "client.h"
#include "mainwindow.h"

#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t) : myteam(t)
{
    setFixedSize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1);
    layout->addWidget(mychat = new QTextEdit(), 0, 1);
    layout->addWidget(myline = new QLineEdit(), 1, 1);
    layout->addWidget(exit = new QPushButton(tr("Exit")), 2, 1);

    myplayers->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mychat->setReadOnly(true);

    connect(exit, SIGNAL(clicked()), SIGNAL(done()));
}

QMenuBar * Client::createMenuBar(MainWindow *w)
{
    (void) w;
    return NULL;
}
