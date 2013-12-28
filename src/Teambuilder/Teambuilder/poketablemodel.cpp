#include "TeambuilderLibrary/modelenum.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "Teambuilder/poketablemodel.h"

using namespace CustomModel;

PokeTableModel::PokeTableModel(Pokemon::gen gen, QObject *parent) : QAbstractTableModel(parent) {
    this->gen = gen;
}

void PokeTableModel::setGen(Pokemon::gen gen) {
    emit layoutAboutToBeChanged();
    this->gen = gen;
    emit layoutChanged();
}

int PokeTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return PokemonInfo::TrueCount();
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

    if (!PokemonInfo::Exists(pokenum, gen) || !PokemonInfo::Released(pokenum, gen)) {
        return QVariant();
    }

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
    case PokegenRole:
        return QVariant::fromValue(gen);
    default:
        break;
    }

    return QVariant();
}

QVariant PokeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        if (!PokemonInfo::Exists(section, gen) || !PokemonInfo::Released(section, gen)) {
            return QVariant();
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (role) {
    case Qt::DisplayRole:
        if (section == 0) {
            return tr("#", "pokemon number");
        } else {
            return tr("Name", "PokemonName");
        }
    case Qt::ToolTipRole:
        if (section == 0) {
            return tr("The dex number of the pokemon");
        } else  {
            return tr("The name of the pokemon");
        }
    };
    return QVariant();
}
