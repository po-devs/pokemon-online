#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include "../PokemonInfo/pokemoninfo.h"

class TB_PokemonBody;

class QCompactTable : public QTableWidget
{
    Q_OBJECT
protected:
    int sizeHintForRow ( int row ) const;
public:
    QCompactTable(int row, int column);
};

/* a widget that allows giving a title to another widget */
class QEntitled : public QWidget
{
    Q_OBJECT
private:
    QLabel *m_title;
    QWidget *m_widget;
    QVBoxLayout *m_layout;

public:
    QEntitled(const QString &title = "Title", QWidget *widget = 0);
    void setTitle(const QString &title);
    void setWidget(QWidget *widget);
};

class TeamBuilder : public QWidget
{
    Q_OBJECT
private:
    QPushButton *m_pokemon[6];
    QPushButton *m_trainer;
    QStackedWidget *m_body;
    QWidget *m_trainerBody;
    TB_PokemonBody *m_pbody[6];
    /* the Team of the trainer */
    Team m_team;

    /* makes the signal/slots connections */
    void connectAll();
    /* returns the current zone (0 = trainer, 1-6 = according pokémon) */
    int currentZone() const;
    /* returns the button associated to that zone */
    QPushButton *at(int i);
    Team *team();

private slots:
    void changeBody(int i);

public:
    TeamBuilder(QWidget *parent = 0);
    ~TeamBuilder();
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
    QLabel *pokeimage;
    QCompactTable *movechoice;
    QDefaultLineEdit *m_moves[4];

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons();
    void initMoves();
    void initItems();

    void configureMoves();
    void updateImage();

    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();
private slots:
    void setMove(int moveNum, int moveSlot);
    void setMove(int movenum);
    void moveCellActivated(int cell);
    void moveEntered(const QModelIndex &index);
signals:
    void moveChosen(int movenum);
public slots:
    void setNum(int pokeNum);
public:
    TB_PokemonBody();

    void setPokeTeam(PokeTeam *new_poke);
};

class TB_EVBar : public QGridLayout
{
    Q_OBJECT
public:
    void add_bar(const QString &desc, int num=213, quint8 evs=0);
};

#endif // TEAMBUILDER_H
