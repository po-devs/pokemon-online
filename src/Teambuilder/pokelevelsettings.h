#ifndef POKELEVELSETTINGS_H
#define POKELEVELSETTINGS_H

#include <QFrame>

namespace Ui {
    class PokeLevelSettings;
}

class PokeTeam;

class PokeLevelSettings : public QFrame
{
    Q_OBJECT

public:
    explicit PokeLevelSettings(QWidget *parent = 0);
    ~PokeLevelSettings();

    void setPoke(PokeTeam *poke);
    void updateAll();
private:
    Ui::PokeLevelSettings *ui;

    PokeTeam *m_poke;
    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateGender();
    void setGender();
};

#endif // POKELEVELSETTINGS_H
