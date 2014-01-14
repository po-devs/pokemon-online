#ifndef POKEBODYWIDGET_H
#define POKEBODYWIDGET_H

#include <QWidget>
#include <QTableView>
#include "../../PokemonInfo/pokemoninfo.h"

class TB_EVManager;
class QLineEdit;
class QLabel;
class QAbstractItemModel;
class QToolButton;
class TB_PokeChoice;
class QComboBox;
class TitledWidget;
class QSortFilterProxyModel;

class PokeBodyWidget : public QWidget {
    Q_OBJECT

    friend class TB_PokemonBody;
public:
    PokeBodyWidget(QWidget *upparent, int gen, QAbstractItemModel *itemsModel,
                       QAbstractItemModel *pokeModel, QAbstractItemModel *natureModel);

    void setMovesModel(QAbstractItemModel *model);
    void setItem(int item);
    void setNature(int nature);
    void changeGen(int gen);
    void loadPokemon(PokeTeam &poke);
    void setWidgetNum(int num);
    void setPicture(const QPixmap &picture);
    void setNum(const Pokemon::uniqueId &num);
    void setTypes(int type1, int type2);
    void setGender(int gender);
    void setLevel(int level);
    void setMove(int index, int num);

    void updateEVs();
signals:
    void pokemonChosen(const Pokemon::uniqueId &id);
    void nickChosen(const QString &nick);
    void natureChanged(int nature);
    void itemChanged(int item);
    void moveChosen(int slot, int move);
    void moveChosen(int move);
    void advanceMenuOpen(bool newWindow);
    void EVChanged(int);
private slots:
    void pokemonTextTriggered();
    void setItem(const QString &item);
    void setNature(int plus, int minus);
    void moveEntered(const QModelIndex &index);
    void moveCellActivated(int index);
    void goToAdvanced();
private:
    class MoveList : public QTableView {
    public:
        MoveList(QAbstractItemModel *model=NULL);
        void setModel(QAbstractItemModel *model);
    private:
        QSortFilterProxyModel *filterModel;
    };

    void initPokemons();
    void initMoves();

    MoveList *movechoice;
    QLineEdit *m_moves[4];
    QLineEdit *m_nick;
    QLineEdit *m_pokeedit;
    TitledWidget *itemlabel;
    TB_EVManager *evchoice;
    TB_PokeChoice *pokechoice;
    QComboBox *itemchoice;
    QComboBox *naturechoice;
    QLabel *nature;
    QLabel *num;
    QToolButton *pokeImage;
    QLabel *genderIcon, *level, *type1, *type2, *pokename, *itemicon;
};

#endif // POKEBODYWIDGET_H
