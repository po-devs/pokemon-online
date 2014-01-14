#ifndef POKEBODY_H
#define POKEBODY_H

#include <QWidget>
#include <QTableView>

#include "../../PokemonInfo/pokemonstructs.h"

class TB_Advanced;
class QModelIndex;
class PokeTeam;
class PokeMovesModel;
class PokeBodyWidget;

/* This is the widget displaying the pokemon's info, moves, item, ... */
class TB_PokemonBody : public QObject
{
    Q_OBJECT

public:
    /* upparent is the widget that will import/export stuff */
    TB_PokemonBody(PokeTeam *poke, int num);
    void connectWithAdvanced(TB_Advanced *ptr);

    void updateNum();
    void setNum(Pokemon::uniqueId, bool resetEverything);
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();
    int num() const {return m_num;}

    void setWidget(PokeBodyWidget *widget);
    void updateItem();
    void changeGeneration(int gen);
    bool hasWidget() const;
    QWidget *getWidget();
public slots:
    void setNum(Pokemon::uniqueId pokeNum);
    /* slots used by advanced */
    void updateAbility();
    void updateLevel();
    void updateGender();
    void updateImage();
    void updateEVs();
    void changeForme(Pokemon::uniqueId);

signals:
    void pokeChanged(Pokemon::uniqueId num);
    void nicknameChanged(QString nickname);
    void EVChanged(int stat);
    void natureChanged();
    void itemChanged(int newItem);
    void levelChanged();
    void pokeImageChanged();

private slots:
    void setMove(int moveNum, int moveSlot);
    void setMove(int movenum);
    void setItem(int);
    void setNature(int nature);
    void setNick(const QString &nick);

private:
    int m_num;

    PokeBodyWidget *widget;
    PokeMovesModel *movesModel;
    int m_index;

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons(TB_PokemonBody *copy = NULL);
    void initMoves();

    void updateMoves();

    int gen();
};

#endif // POKEBODY_H
