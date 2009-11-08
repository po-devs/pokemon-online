#include "client.h"
#include "mainwindow.h"

#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t) : myteam(t), myrelay()
{
    setFixedSize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1);
    layout->addWidget(mychat = new QTextEdit(), 0, 1, 1, 2);
    layout->addWidget(myline = new QLineEdit(), 1, 1, 1, 2);
    layout->addWidget(myexit = new QPushButton(tr("&Exit")), 2, 1);
    layout->addWidget(mysender = new QPushButton(tr("&Send")), 2, 2);

    myplayers->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    myplayers->setSortingEnabled(true);
    mychat->setReadOnly(true);

    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));

    initRelay();

    relay().connectTo("localhost", 5080);
}

void Client::sendText()
{
    relay().sendMessage(myline->text());
    myline->clear();
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
    connect(&relay(), SIGNAL(connected()), SLOT(connected()));
    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(messageReceived(QString)), SLOT(messageReceived(QString)));
    connect(&relay(), SIGNAL(playerReceived(Player)), SLOT(playerLogIn(Player)));
}

void Client::messageReceived(const QString &mess)
{
    printLine(mess);
}

void Client::errorFromNetwork(int errno, const QString &errorDesc)
{
    QMessageBox::critical(this, tr("Error while connected to server"), tr("Received error n°%1: %2").arg(errno).arg(errorDesc));
}

void Client::connected()
{
    printLine(tr("Connected to Server!"));

    relay().login(team()->trainerNick(), team()->trainerNick());
    relay().sendTeam(*team());
}

void Client::disconnected()
{
    printLine(tr("Disconnected from Server!"));
}

TrainerTeam* Client::team()
{
    return myteam;
}

Analyzer &Client::relay()
{
    return myrelay;
}

QTextEdit *Client::mainChat()
{
    return mychat;
}

void Client::playerLogIn(const Player& p)
{
    myplayersinfo.insert(p.id, p.team);
    printLine(tr("%1 logged in.").arg(p.team.name));

    myplayers->addItem(p.team.name);
}

void Client::printLine(const QString &line)
{
    mainChat()->insertPlainText(line + "\n");
}

QDataStream & operator >> (QDataStream &in, Player &p)
{
    in >> p.id;
    in >> p.team;

    return in;
}

QDataStream & operator << (QDataStream &out, const Player &p)
{
    out << p.id;
    out << p.team;

    return out;
}
