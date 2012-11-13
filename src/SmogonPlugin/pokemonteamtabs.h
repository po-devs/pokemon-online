#ifndef POKEMONTEAMTABS_H
#define POKEMONTEAMTABS_H

#include "../PokemonInfo/pokemoninfo.h"
#include "pokemontab.h"
#include <QLabel>
#include <QDialog>
#include <QTabWidget>

class PokemonTeamTabs : public QTabWidget{
    Q_OBJECT
public:
    PokemonTeamTabs(QString saveFilePath);
    void addPokeTab(PokemonTab* page, const QString& label);
public slots:
    Team* getTeam();
private:
    QList<PokemonTab*> *pokemonTabs;
    QString filePath;
};

#endif // POKEMONTEAMTABS_H
