#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <QtGui>
#include "../pokemoninfo.h"

/* The static almighty PokemonInfo class, which provides any info we need on Pokemons */
static PokemonInfo *PkInfo;
/* Same for Items */
static ItemInfo *ItInfo;

class TB_PokemonBody;

class QCompactTable : public QTableWidget
{
    Q_OBJECT
protected:
    int sizeHintForRow ( int row ) const;
public:
    QCompactTable(int row, int column);
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

    /* makes the signal/slots connections */
    void connectAll();
    /* returns the current zone (0 = trainer, 1-6 = according pokémon) */
    int currentZone() const;
    /* returns the button associated to that zone */
    QPushButton *at(int i);

private slots:
    void changeBody(int i);

public:
    TeamBuilder(QWidget *parent = 0);
    ~TeamBuilder();
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

class TB_PokemonBody : public QWidget
{
    Q_OBJECT
private:
    QCompactTable *pokechoice;
    QComboBox *itemchoice;
    QLabel *pokeimage;

    void loadPokemons();
    void loadItems();
public:
    TB_PokemonBody();
};

class TB_EVBar : public QGridLayout
{
    Q_OBJECT
public:
    void add_bar(const QString &desc, int num=213, quint8 evs=0);
};

#endif // TEAMBUILDER_H
