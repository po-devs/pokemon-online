#ifndef POKEMOVESMODEL_H
#define POKEMOVESMODEL_H

#include <QAbstractTableModel>
#include "../PokemonInfo/pokemonstructs.h"
#include "modelenum.h"

class PokeMovesModel : public QAbstractTableModel
{
public:
    PokeMovesModel(const Pokemon::uniqueId &id, Pokemon::gen gen = Pokemon::gen(), QObject *parent=0);
    void setPokemon(const Pokemon::uniqueId &id, Pokemon::gen gen = Pokemon::gen());

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

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
    void loadData();

    Pokemon::uniqueId id;
    Pokemon::gen gen;

    QMap<QString, QPair<int, QString> > storage;
    QList<QString> names;
};

#endif // POKEMOVESMODEL_H
