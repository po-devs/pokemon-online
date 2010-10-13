#include "teamsaver.h"
#include "pokemonstructs.h"

TeamSaver::TeamSaver(TrainerTeam *t)
{
    this->t = t;
}

void TeamSaver::fileNameReceived(const QString &name)
{
    QSettings s;
    s.setValue("team_location", name);
    t->saveToFile(name);

    deleteLater();
}

void TeamSaver::fileNameReceivedL(const QString &name)
{
    QSettings s;
    s.setValue("team_location", name);
    t->loadFromFile(name);

    deleteLater();
}
