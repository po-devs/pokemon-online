#ifndef POKEBODY_H
#define POKEBODY_H

#include <QWidget>
#include <QTableView>

#include "../../PokemonInfo/pokemonstructs.h"

class TeamBuilder;
class QAbstractItemModel;
class TB_Advanced;
class QModelIndex;
class QToolButton;
class TB_PokeChoice;
class QComboBox;
class TitledWidget;
class PokeTeam;
class TB_EVManager;
class QLineEdit;
class QLabel;
class PokeMovesModel;

/* This is the widget displaying the pokemon's info, moves, item, ... */
class TB_PokemonBody : public QWidget
{
    Q_OBJECT

public:
    /* upparent is the widget that will import/export stuff */
    TB_PokemonBody(QWidget *upparent, PokeTeam *poke, int num, int gen, QAbstractItemModel *itemsModel,
                   QAbstractItemModel *pokeModel, QAbstractItemModel *natureModel);
    void connectWithAdvanced(TB_Advanced *ptr);

    void updateNum();
    void setNum(Pokemon::uniqueId, bool resetEverything);
    /* getting the pokemon of the team corresponding to the body */
    PokeTeam *poke();
    int num() const {return m_num;}

    void changeGeneration(int gen);
    void updateItem();

public slots:
    void setNum(Pokemon::uniqueId pokeNum);
    void setPokeByNick();
    /* slots used by advanced */
    void updateImage();
    void updateGender();
    void updateLevel();
    void updateEVs();
    void updateAbility();
    void changeForme(Pokemon::uniqueId);

signals:
    void moveChosen(int movenum);
    void pokeChanged(Pokemon::uniqueId num);
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
    void moveEntered(const QModelIndex &index);
    void setItem(const QString &item);
    void setNature(int nature);
    void goToAdvanced();
    void setNick(const QString &nick);
    void editNature(int up, int down);

private:
    TB_PokeChoice *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    QLabel *nature;
    QToolButton *pokeImage;
    QLabel *genderIcon, *level, *type1, *type2, *pokename, *itemicon;
    int m_num;

    class MoveList : public QTableView {
    public:
        MoveList(QAbstractItemModel *model);
    };

    MoveList *movechoice;
    QLineEdit *m_moves[4];
    QLineEdit *m_nick;
    QLineEdit *m_pokeedit;
    TitledWidget *itemlabel;
    TB_EVManager *evchoice;

    PokeMovesModel *movesModel;
    int m_index;
    int gen;

    /* the pokemon of the team corresponding to the body */
    PokeTeam *m_poke;

    void initPokemons(TB_PokemonBody *copy = NULL);
    void initMoves();

    void updateMoves();
    void updateNickname();
    void updateNature();
    void updatePokeChoice();
    void updateTypes();

    bool advancedOpen();
};

#endif // POKEBODY_H
