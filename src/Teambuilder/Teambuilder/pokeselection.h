#ifndef POKESELECTION_H
#define POKESELECTION_H

#include <QWidget>
#include "../PokemonInfo/pokemon.h"
#include "../PokemonInfo/geninfo.h"

namespace Ui {
class PokeSelection;
}

class AdvancedSearch;
class QAbstractItemModel;
class QAbstractProxyModel;
class PokeTableModel;
class QModelIndex;

class PokeSelection : public QWidget
{
    Q_OBJECT
    
public:
    explicit PokeSelection(Pokemon::uniqueId pokemon, QAbstractItemModel *pokemonModel);
    ~PokeSelection();
    void show();
signals:
    void pokemonChosen(Pokemon::uniqueId);
    void shinySelected(bool shiny);
private slots:
    void updateTypes();
    void updateSprite();
    void setNum(const Pokemon::uniqueId&);
    void setPokemon(const QModelIndex &);
    void changeForme(int);
    void finish();
    void toggleSearchWindow();
private:
    Ui::PokeSelection *ui;
    QAbstractItemModel *sourceModel;
    QAbstractProxyModel *proxy;

    Pokemon::uniqueId num() const {
        return m_num;
    }

    Pokemon::gen getGen();

    Pokemon::uniqueId m_num;

    //ui stuff
    AdvancedSearch *search;
    int oldwidth, newwidth, oldx;
};

#endif // POKESELECTION_H
