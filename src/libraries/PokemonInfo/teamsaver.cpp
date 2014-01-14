#include "teamsaver.h"
#include "pokemonstructs.h"

TeamSaver::TeamSaver(Team *t) :t(t)
{
}

void TeamSaver::fileNameReceived(const QString &name)
{
    t->saveToFile(name);

    QSettings s;
    s.setValue("Teams/Folder", t->folder());

    deleteLater();
}

void TeamSaver::fileNameReceivedL(const QString &name)
{
    t->loadFromFile(name);

    QSettings s;
    s.setValue("Teams/Folder", t->folder());

    deleteLater();
}
