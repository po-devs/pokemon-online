#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>

class TB_PokemonBody;
class TB_TrainerBody;
class TB_EVManager;
class TB_Advanced;

class TrainerTeam;
class Team;
class PokeTeam;
class MainWindow;

class QCompactTable;

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

class DockAdvanced;

/* The Teambuilder!! */
class TeamBuilder : public QWidget
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

private slots:
    void changeBody(int i);
    void setIconForPokeButton();
    void setNicknameIntoButton(QString nickname);
    void advancedClicked(int index);
    void advancedDestroyed();
    void indexNumPokemonChangedForAdvanced(int pokeNum);
    void updateDataBody(int indexStacked);

public slots:
    void saveTeam();
    void loadTeam();
    void clickOnDone();
    void updateTeam();

signals:
    void done();
    void showDockAdvanced(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation);

public:
    TeamBuilder(TrainerTeam *team);
    ~TeamBuilder();

    TrainerTeam *trainerTeam();
    Team * getTeam()const;

    /* Create a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainWindow *w);
    void createDockAdvanced();
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
private:
    QCompactTable *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    QLabel *pokeImage, *genderIcon, *level;
    QCompactTable *movechoice;
    QLineEdit *m_moves[4];
    QLineEdit *m_nick;
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
    void updateNickname();
    void updateItem();
    void updateNature();

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
    void setAdvancedOpenToFalse();
    void setNick(const QString &nick);
signals:
    void moveChosen(int movenum);
    void pokeChanged(int pokenum);
    void nicknameChanged(QString nickname);
    void advanced(int index);
public slots:
    void setNum(int pokeNum);
    void updateAdvanced();
public:
    TB_PokemonBody(PokeTeam *poke);

    void updateNum();
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();

};

/* Manages the EV bars, inside the TB_PokemonBody */
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

#endif // TEAMBUILDER_H
