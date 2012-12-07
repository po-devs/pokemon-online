#include "../Utilities/otherwidgets.h"
#include "channel.h"
#include "player.h"

QNickValidator *Channel::checker = NULL;

unsigned int qHash(const QPointer<Player> &pl)
{
    return qHash(pl.data());
}

Channel::Channel(const QString &name, int id) : name(name), logDay(0), id(id) {
    QDir d;
    if(!d.exists("logs/chat/" + name)) {
        d.mkpath("logs/chat/" + name);
    }
}

void Channel::log(const QString &message) {
    if(!logfile.isOpen() || logDay != QDate::currentDate().day()) {
        if(logfile.isOpen()) {
            logfile.close();
        }
        QString date = QDate::currentDate().toString("yyyy-MM-dd");
        QString filename = "logs/chat/"+name+"/"+date+".txt";
        logDay = QDate::currentDate().day();
        logfile.setFileName(filename);
        logfile.open(QFile::WriteOnly | QFile::Append | QFile::Text);
    }
    logfile.write(QString("(%1) %2\n").arg(QTime::currentTime().toString("hh:mm:ss"), message).toUtf8());
    logfile.flush();
}

bool Channel::validName(const QString &name) {
    return checker->validate(name) == QValidator::Acceptable;
}

Channel::~Channel()
{
    foreach(Player *p, players) {
        p->removeChannel(id);
    }
    foreach(Player *p, disconnectedPlayers) {
        if (p) {
            p->removeChannel(id);
        }
    }

    if(logfile.isOpen()) {
        logfile.close();
    }
}
