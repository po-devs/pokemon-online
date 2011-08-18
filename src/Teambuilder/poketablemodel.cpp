#include "poketablemodel.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "modelenum.h"

using namespace CustomModel;

PokeTableModel::PokeTableModel(int gen, QObject *parent) : QAbstractTableModel(parent) {
    this->gen = gen;
}

void PokeTableModel::setGen(int gen) {
    emit layoutAboutToBeChanged();
    this->gen = gen;
    emit layoutChanged();
}

int PokeTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return PokemonInfo::TrueCount(gen);
}

int PokeTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return 2;
}

QVariant PokeTableModel::data(const QModelIndex &index, int role) const {
    int pokenum = index.row();
    int column = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (column == 0) {
            return QString::number(pokenum).rightJustified(3,'0');
        } else {
            return PokemonInfo::Name(pokenum);
        }
    case Qt::DecorationRole:
        /* Todo: test with the icon displayed instead of the num,
          see which is better */
        //return PokemonInfo::Icon(pokenum);
        return QVariant();
    case Qt::ToolTipRole:
        return PokemonInfo::Desc(pokenum, Version::HeartGold);
    case PokenumRole:
        return pokenum;
    case PokenameRole:
        return PokemonInfo::Name(pokenum);
    case PokeimageRole:
        return PokemonInfo::Picture(pokenum, gen);
    default:
        break;
    }

    return QVariant();
}
