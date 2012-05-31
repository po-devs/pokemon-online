#ifndef POKEDEX_H
#define POKEDEX_H

#include <QtGui>
#include "../PokemonInfo/pokemonstructs.h"

class TB_PokeChoice;
class GridBox;
class QImageButtonLR;
class QCompactTable;
class TypeChart;
class AdvancedSearch;
class PokeMovesModel;

class Pokedex : public QWidget
{
    Q_OBJECT
public:
    Pokedex(QWidget *parent, QAbstractItemModel *pokeModel);
public slots:
    void showTypeChart();
private:
    QPointer<TypeChart> typeChart;
};

class BigOpenPokeBall : public QLabel
{
    Q_OBJECT
public:
    BigOpenPokeBall();

    void update();
    bool shiny() const;
public slots:
    void changeToPokemon(Pokemon::uniqueId poke);
    void updatePicture();
    void changeToNext();
    void changeToPrevious();
signals:
    void pokemonChanged(Pokemon::uniqueId);
private:
    Pokemon::uniqueId currentPoke;
    QPushButton *evo, *formes;
    QLabel *num, *name;
    QLabel *specy;
    QLabel *height, *weight;
    QLabel *type1, *type2;
    QLabel *genderN, *genderF, *genderM;
    GridBox *front, *back;
    QCheckBox *shinyBox;
};

class PokedexBody : public QFrame
{
    Q_OBJECT

    enum {
        SortByNum = 0,
        SortByAlph = 1
    };
public:
    PokedexBody(QAbstractItemModel *model);

    void changeToPokemon(Pokemon::uniqueId poke);
public slots:
    void sortByColumn(int col);
    void changeToPokemonFromExt(Pokemon::uniqueId);
    void openAdvancedSearch();
signals:
    void pokeChanged(Pokemon::uniqueId newPoke);
private slots:
    void changeToPokemon(const QString &);
    void changePokemon();
    void changePokemonFromRow(const QModelIndex &index);
private:
    Pokemon::uniqueId currentPoke;
    TB_PokeChoice * pokeList;
    QLineEdit *pokeEdit;
    QPointer<AdvancedSearch> aSearch;
};

class ProfileTab : public QFrame
{
    Q_OBJECT
public:
    ProfileTab();
public slots:
    void changeDesc (Pokemon::uniqueId poke);
private:
    QLabel *hgDesc, *ssDesc, *ptDesc;
    QLabel *abs[3];
};

class MoveTab : public QFrame
{
    Q_OBJECT
public:
    MoveTab();
public slots:
    void changePoke(Pokemon::uniqueId poke);
private:
    QTableView *moves;
    PokeMovesModel *movesModel;
};

class StatTab: public QFrame
{
    Q_OBJECT
public:
    StatTab();
public slots:
    void changePoke(Pokemon::uniqueId poke);
private slots:
    void increaseBoost();
    void decreaseBoost();
private:
    Pokemon::uniqueId poke;
    QLabel *min[6];
    QLabel *max[6];
    QImageButtonLR *buttons[6];
    QProgressBar *baseStats[6];
    int boost[6];
    QTabWidget *weakImmTab;
};

/* Just used for CSS */
class PokeBallDescBox : public QFrame
{
    Q_OBJECT
public:
    PokeBallDescBox() {}
    PokeBallDescBox(QWidget *parent) : QFrame(parent) {};
};

/* A box specially made to display 80*80 avatars */
/* Ripped off from AvatarBox (teambuilder.h) */
class GridBox : public QLabel {
public:
    GridBox(const QPixmap &pic=QPixmap(), bool shiftToBottom = false);

    void changePic(const QPixmap &pic);
protected:
    QLabel *underLying;
};

/* A pokeball icon and a text */
class PokeBallText : public QWidget
{
public:
    PokeBallText(const QPixmap &pic, const QString &text);
};

class TypeText : public QWidget
{
public:
    TypeText(int type, const QString &text);
};


class TypeChart : public QWidget
{
    Q_OBJECT
public:
    TypeChart(QWidget *parent);
};

class AdvancedSearch : public QWidget
{
    Q_OBJECT
public:
    AdvancedSearch();
signals:
    void pokeSelected(Pokemon::uniqueId);
private slots:
    void search();
    void pokeClicked(QModelIndex);
private:
    QCheckBox *typeBoxes[2];
    QComboBox *typeCb[2];
    QComboBox *abilityCb;
    QComboBox *statSymbols[6];
    QLineEdit *stats[6];
    QLineEdit *move[4];
    QListWidget *results;
};

#endif // POKEDEX_H
