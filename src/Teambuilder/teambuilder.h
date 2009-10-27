#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include "../PokemonInfo/pokemoninfo.h"
#include <QMenuBar>
#include "otherwidgets.h"

class TB_PokemonBody;
class TB_TrainerBody;
class TB_EVManager;
class TB_Advanced;

/* validator for the nicks */
class QNickValidator : public QValidator
{
    Q_OBJECT
public:
    QNickValidator(QWidget *parent);

    bool isBegEndChar(QChar ch) const;
    void fixup(QString &input) const;
    State validate(QString &input, int &pos) const;
};

class TeamBuilder : public QCenteredWidget
{
    Q_OBJECT
private:
    QPushButton *m_pokemon[6];
    QPushButton *m_trainer;
    QStackedWidget *m_body;
    TB_TrainerBody *m_trainerBody;
    TB_PokemonBody *m_pbody[6];
    /* the Team of the trainer */
    TrainerTeam *m_team;

    /* returns the button associated to that zone */
    QPushButton *at(int i);
    Team *team();
    TrainerTeam *trainerTeam();

    void updatePokemon(int index);
    void updateTrainer();
    /* makes the signal/slots connections */
    void connectAll();
    /* returns the current zone (0 = trainer, 1-6 = according pokémon) */
    int currentZone() const;
    /*...*/
    TB_PokemonBody *pokebody(int index);
    TB_TrainerBody *trainerbody();

private slots:
    void changeBody(int i);
    void setIconForPokeButton(int num);

public slots:
    void saveTeam();
    void loadTeam();
    void done();
    void updateTeam();

public:
    TeamBuilder(TrainerTeam *team);
    ~TeamBuilder();
};

class TB_TrainerBody : public QWidget
{
    Q_OBJECT
private:
    QLineEdit *m_nick;
    QTextEdit *m_winMessage, *m_loseMessage, /* *m_drawMessage, */ *m_trainerInfo;
public:
    TB_TrainerBody(TeamBuilder *parent);

    void updateTrainer();
};

class TB_PokemonBody : public QWidget
{
    Q_OBJECT

    enum Column
    {
	Type=0,
	Name=1,
	Learning,
	PP,
	Pow,
	Acc,
	Category,
	LastColumn
    };
private:
    QCompactTable *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    QLabel *pokeImage, *genderIcon, *level;
    QCompactTable *movechoice;
    QLineEdit *m_moves[4];
    TB_EVManager *evchoice;
    TB_Advanced *m_adv;
    int m_index;

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons();
    void initMoves();
    void initItems();

    void configureMoves();
    void updateImage();
    void updateGender();
    void updateLevel();
    void updateMoves();
    void updateEVs();

    bool advancedOpen();
    TB_Advanced *advanced();

private slots:
    void setMove(int moveNum, int moveSlot);
    void setMove(int movenum);
    void moveCellActivated(int cell);
    void moveEntered(int row);
    void setItem(const QString &item);
    void setNature(int nature);
    void goToAdvanced();
    void updateAdvanced();
    void setAdvancedOpenToFalse();
signals:
    void moveChosen(int movenum);
    void pokeChanged(int pokenum);
    void changeIconForPokeButton(int num);
public slots:
    void setNum(int pokeNum);
public:
    TB_PokemonBody(PokeTeam *poke);

    void updateNum();
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();

};

class TB_EVManager : public QGridLayout
{
    Q_OBJECT
private:
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLabel *m_evs[6];
    QSlider *m_mainSlider;
    PokeTeam *m_poke;

    PokeTeam *poke();
    QSlider *slider(int stat);
    const QSlider *slider(int stat) const;
    QLabel *evLabel(int stat);
    QLabel *statLabel(int stat);
    /* the reverse of slider(int) */
    int stat(QObject *sender) const;
public:
    TB_EVManager(PokeTeam *poke);

    void updateEVs();
    void updateEV(int stat);
    void updateMain();
public slots:
    void EVChanged(int newvalue);
};

QDataStream & operator << (QDataStream & out,const Team & team);
QDataStream & operator << (QDataStream & out,const PokeTeam & Pokemon);
QDataStream & operator << (QDataStream & out,const TrainerTeam & trainerTeam);

QDataStream & operator >>(QDataStream & in,Team & team);
QDataStream & operator >>(QDataStream & in,PokeTeam & Pokemon);
QDataStream & operator >>(QDataStream & in,TrainerTeam & trainerTeam);

#endif // TEAMBUILDER_H
