#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include <QPair>
#include "../Utilities/otherwidgets.h"

class TB_PokemonBody;
class TB_TrainerBody;
class TB_EVManager;
class DockAdvanced;
class pokeButton;
class TB_Advanced;

class TrainerTeam;
class Team;
class PokeTeam;
class MainEngine;

class TeamImporter : public QWidget
{
    Q_OBJECT
public:
    TeamImporter();
signals:
    void done(const QString&);
public slots:
    void done();
private:
    QPlainTextEdit *mycontent;
};

/* The Teambuilder!! */
class TeamBuilder : public QWidget
{
    Q_OBJECT

    friend class DockAdvanced;
private slots:
    void changeBody(int i);
    void setIconForPokeButton();
    void setNicknameIntoButton(QString nickname);
    void advancedClicked(int index);
    void advancedDestroyed();
    void indexNumPokemonChangedForAdvanced(int pokeNum);
    void changePokemonOrder(QPair<int /*pokemon1*/, int /*pokemon2*/>echange);

public slots:
    void saveTeam();
    void loadTeam();
    void newTeam();
    void clickOnDone();
    void updateTeam();
    void importFromTxt();
    void importDone(const QString &text);

signals:
    void done();
    void showDockAdvanced(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation);

public:
    TeamBuilder(TrainerTeam *team);
    ~TeamBuilder();

    TrainerTeam *trainerTeam();
    Team * getTeam()const;

    /* Create a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);
    void createDockAdvanced();

private:
    pokeButton *m_pokemon[6];
    QPushButton *m_trainer;
    QStackedWidget *m_body;
    TB_TrainerBody *m_trainerBody;
    TB_PokemonBody *m_pbody[6];
    /* the Team of the trainer */
    TrainerTeam *m_team;

    QPointer<QWidget> m_import;

    /* returns the button associated to that zone */
    QPushButton *at(int i);
    Team *team();

    void updatePokemon(int index);
    void updateTrainer();
    /* makes the signal/slots connections */
    void connectAll();
    /* returns the current zone (0 = trainer, 1-6 = according pokémon) */
    int currentZone() const;
    /*...*/

    TB_PokemonBody *pokebody(int index);
    TB_TrainerBody *trainerbody();

    //dockAdvanced
    DockAdvanced * m_dockAdvanced;
    DockAdvanced * dockAdvanced() const;
};

/* This is the widget displaying a trainer's info */

class TB_TrainerBody : public QWidget
{
    Q_OBJECT
private:
    QLineEdit *m_nick;
    QTextEdit *m_winMessage, *m_loseMessage, /* *m_drawMessage, */ *m_trainerInfo;

     TrainerTeam *m_team;
     TrainerTeam* trainerTeam();
 private slots:
     void changeTrainerInfo();
     void setTrainerNick(const QString &);
     void changeTrainerWin();
     void changeTrainerLose();
public:
    TB_TrainerBody(TeamBuilder *parent);

    void updateTrainer();
};

/* This is the widget displaying the pokemon's info, moves, item, ... */
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

signals:
    void moveChosen(int movenum);
    void pokeChanged(int pokenum);
    void nicknameChanged(QString nickname);
    void advanced(int index);
    void EVChanged(int stat);
    void natureChanged();
public slots:
    void setNum(int pokeNum);
    void setPokeByNick();
    /* slots used by advanced */
    void updateImage();
    void updateGender();
    void updateLevel();
    void updateEVs();
    void changeForm(int pokenum);
public:
    TB_PokemonBody(PokeTeam *poke);
    void connectWithAdvanced(TB_Advanced *ptr);
    inline void changeSourcePoke(PokeTeam *poke) {m_poke = poke;}

    void updateNum();
    void setNum(int pokeNum, bool resetEverything);
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();
private:
    QCompactTable *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    QLabel *pokeImage, *genderIcon, *level, *type1, *type2;

    class MoveList : public QCompactTable {
    public:
        MoveList(int a,int b):QCompactTable(a,b) {}
    protected:
        bool event(QEvent *event);
    };

    MoveList *movechoice;
    QLineEdit *m_moves[4];
    QLineEdit *m_nick;
    QLineEdit *m_pokeedit;
    TB_EVManager *evchoice;

    int m_index;

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons();
    void initMoves();
    void initItems();

    void configureMoves();

    void updateMoves();
    void updateNickname();
    void updateItem();
    void updateNature();
    void updatePokeChoice();
    void updateTypes();

    bool advancedOpen();
private slots:
    void setMove(int moveNum, int moveSlot);
    void setMove(int movenum);
    void moveCellActivated(int cell);
    void moveEntered(int row);
    void setItem(const QString &item);
    void setNature(int nature);
    void goToAdvanced();
    void setNick(const QString &nick);
};

/* Manages the EV bars, inside the TB_PokemonBody */
class TB_EVManager : public QGridLayout
{
    Q_OBJECT
private:
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLineEdit *m_evs[6];
    QSlider *m_mainSlider;
    PokeTeam *m_poke;

    PokeTeam *poke();
    QSlider *slider(int stat);
    const QSlider *slider(int stat) const;
    QLineEdit *evLabel(int stat);
    const QLineEdit *evLabel(int stat) const;
    QLabel *statLabel(int stat);
    /* the reverse of slider(int), evlabel(int) */
    int stat(QObject *sender) const;
public:
    TB_EVManager(PokeTeam *poke);

    void updateEVs();
    void updateEV(int stat);
    void updateMain();
public slots:
    void changeEV(int newvalue);
    void changeEV(const QString &newvalue);
signals:
    void EVChanged(int stat);
};

#endif // TEAMBUILDER_H
