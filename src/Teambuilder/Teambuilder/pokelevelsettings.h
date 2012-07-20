#ifndef POKELEVELSETTINGS_H
#define POKELEVELSETTINGS_H

#include <QFrame>

class QRadioButton;

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

signals:
    void levelUpdated();
    void genderUpdated();

public slots:
    void changeLevel(int newLevel);
    void changeGender();
    void changeAbility();

    void updateGender();
    void updateAbility();
private:
    Ui::PokeLevelSettings *ui;

    PokeTeam *m_poke;
    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void setGender();
    void setAbilities();

    QRadioButton *m_abilities[3];
};

#endif // POKELEVELSETTINGS_H
