#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include <QPair>

#include "../Utilities/otherwidgets.h"
#include "centralwidget.h"

class TB_PokemonBody;
class TB_TrainerBody;
class TB_PokemonBoxes;
class Pokedex;
class TB_TeamBody;
class TB_EVManager;
class DockAdvanced;
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

/* A box specially made to display 80*80 avatars */
class AvatarBox : public QLabel {
public:
    AvatarBox(const QPixmap &pic=QPixmap());

    void changePic(const QPixmap &pic);
protected:
    static QPixmap *background;
    QLabel *underLying;
};

/* Titles and such with the blue pokeball in front */
class Pokeballed : public QWidget {
public:
    Pokeballed(QWidget *w);
protected:
    Pokeballed();
    void init(QWidget *w);
public:
    static QPixmap *pokeball;
};

class TitledWidget : public QWidget {
public:
    TitledWidget(const QString &title, QWidget *w);
};

/* The Teambuilder!! */
class TeamBuilder : public QLabel, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(TrainerTeam *team);
    ~TeamBuilder();

    TrainerTeam *trainerTeam();
    Team *team();

    /* Create a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

public slots:
    void saveTeam();
    void loadTeam();
    void newTeam();
    void pokeChanged(int poke);
    void clickOnDone();
    void updateAll();
    void importFromTxt();
    void exportToTxt();
    void importDone(const QString &text);
    void showNoFrame();
    void changeItemDisplay(bool allItems);

private:
    enum StackWidgets {
        TrainerW=0,
        TeamW=1,
        BoxesW=2,
        PokedexW=3,
        LastW
    };

private slots:
    void changeToTrainer();
    void changeToTeam();
    void changeToBoxes();
    void changeToPokedex();
    void changeZone();
    void genChanged();

signals:
    void done();
private:
    QStackedWidget *m_body;
    TB_TrainerBody *m_trainerBody;
    TB_TeamBody *m_teamBody;
    TB_PokemonBoxes *m_boxes;
    Pokedex *m_pokedex;
    QAction *gen3, *gen4;

    QImageButton *buttons[LastW];
    QLabel *currentZoneLabel;
    /* the Team of the trainer */
    TrainerTeam *m_team;

    QPointer<QWidget> m_import;

    void updateTrainer();
    void updateTeam();
    void updateBox();

    TB_TrainerBody *trainerbody();

    bool modified[6];
};

/* This is the widget displaying a trainer's info */

class TB_TrainerBody : public QWidget
{
    Q_OBJECT
public:
    TB_TrainerBody(TeamBuilder *parent);

    void updateTrainer();
private slots:
    void changeTrainerInfo();
    void setTrainerNick(const QString &);
    void changeTrainerWin();
    void changeTrainerLose();
    void changeTrainerAvatar(int);
    void changeTrainerColor();
private:
    QLineEdit *m_nick;
    QPlainTextEdit *m_winMessage, *m_loseMessage, *m_trainerInfo;
    QPushButton *m_colorButton;
    AvatarBox *m_avatar;
    QSpinBox *m_avatarSelection;

    TrainerTeam *m_team;
    TrainerTeam* trainerTeam();
};

class TeamPokeButton : public QPushButton
{
    Q_OBJECT
public:
    TeamPokeButton(int num, int poke=0, int level=100, int item = 0);

    void changeInfos(int poke=0, int level=100, int item = 0);
    int num() const {return m_num;}
signals:
    void changePokemonOrder(QPair<int,int> exchange);
    void changePokemonBase(int num, int poke);

protected:
    static QPixmap * teamBoxBall;

    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void startDrag();
private:
    QLabel *pokeIcon, *level, *itemIcon;
    int m_num;

    QPoint startPos;
};

class TB_TeamBody: public QWidget
{
    Q_OBJECT
public:
    TB_TeamBody(TeamBuilder *parent, int gen=4);
    void updateTeam();
    void updatePoke(int num);
    void changeGeneration(int gen);
private slots:
    void changeIndex();
    void updateButton();
    void changePokemonOrder(QPair<int,int>);
    void changePokemonBase(int slot, int num);
    void advancedClicked(int index, bool separateWindow);
    void advancedDestroyed();
    void indexNumChanged(int pokeNum);
private:
    TeamPokeButton *pokeButtons[6];
public:
    TB_PokemonBody *pokeBody[6];
private:
    QStackedWidget *body;

    QSplitter *splitter;
    bool advSepWindow;
    DockAdvanced * m_dockAdvanced;
    DockAdvanced * dockAdvanced() const {
        return m_dockAdvanced;
    }
    void createDockAdvanced(bool sepWindow);

    TrainerTeam *m_team;
    TrainerTeam* trainerTeam() {
        return m_team;
    }

    int gen;

    friend class DockAdvanced;
};

/* The list of pokemons */
class TB_PokeChoice : public QCompactTable
{
    Q_OBJECT

public:
    TB_PokeChoice(int gen, bool missingno);

    void changeGen(int gen);
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    void startDrag();

    QPoint startPos;
    QTableWidgetItem * itemForDrag;

    bool missingno;
    int gen;
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
public:
    TB_PokemonBody(TeamBuilder *upparent, PokeTeam *poke, int num, int gen=4);
    void connectWithAdvanced(TB_Advanced *ptr);

    void updateNum();
    void setNum(int pokeNum, bool resetEverything);
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();
    int num() const {return m_num;}

    void reloadItems(bool showAllItems);
    void changeGeneration(int gen);

public slots:
    void setNum(int pokeNum);
    void setPokeByNick();
    /* slots used by advanced */
    void updateImage();
    void updateGender();
    void updateLevel();
    void updateEVs();
    void changeForme(int pokenum);

signals:
    void moveChosen(int movenum);
    void pokeChanged(int pokenum);
    void nicknameChanged(QString nickname);
    void advanced(int index, bool separateWindow);
    void EVChanged(int stat);
    void natureChanged();
    void itemChanged(int newItem);
    void levelChanged();
    void pokeImageChanged();

private slots:
    void setMove(int moveNum, int moveSlot);
    void setMove(int movenum);
    void moveCellActivated(int cell);
    void moveEntered(int row);
    void setItem(const QString &item);
    void setNature(int nature);
    void goToAdvanced();
    void setNick(const QString &nick);
    void editNature(int up, int down);

private:
    TB_PokeChoice *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    AvatarBox *pokeImage;
    QLabel *genderIcon, *level, *type1, *type2, *pokename, *itemicon;
    int m_num;

    class MoveList : public QCompactTable {
    public:
        MoveList();
    protected:
        bool event(QEvent *event);
    };

    MoveList *movechoice;
    QLineEdit *m_moves[4];
    QLineEdit *m_nick;
    QLineEdit *m_pokeedit;
    TB_EVManager *evchoice;

    int m_index;
    int gen;

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons(TB_PokemonBody *copy = NULL);
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
};

/* Manages the EV bars, inside the TB_PokemonBody */
class TB_EVManager : public QWidget
{
    Q_OBJECT
private:
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLineEdit *m_evs[6];
    QSlider *m_mainSlider;
    PokeTeam *m_poke;
    QLabel * m_mainLabel;
    QImageButtonLR *natureButtons[5];

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


    /* Nature selectors */

    int myStatUp;
    int myStatDown;

    void updateEVs();
    void updateEV(int stat);
    void updateMain();
    void updateNatureButtons();
public slots:
    void changeEV(int newvalue);
    void changeEV(const QString &newvalue);
    void checkNButtonR();
    void checkNButtonL();
signals:
    void EVChanged(int stat);
    void natureChanged(int up, int down);
};


#endif // TEAMBUILDER_H
