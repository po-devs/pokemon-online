#include "PokemonInfo/pokemoninfo.h"
#include "theme.h"
#include "pokemovesmodel.h"

template <class T, class U>
QHash<typename T::value_type, U> map_container_with_value(T container, const U & value)
{
    QHash<typename T::value_type, U> ret;

    foreach(typename T::value_type val, container)
        ret.insert(val, value);

    return ret;
}

PokeMovesModel::PokeMovesModel(const Pokemon::uniqueId &id, Pokemon::gen gen, QObject *parent) : QAbstractTableModel(parent), id(id), gen(gen)
{
    loadData();
}

QHash<int, QString> getMoves(const Pokemon::uniqueId &num, Pokemon::gen gen, bool root = true) {
    QHash<int, QString> ret;
    if (gen.num != 1 && gen.num != 3) {
        ret = getMoves(num, Pokemon::gen(gen.num-1, GenInfo::NumberOfSubgens(gen.num-1)-1), false);
    }
    return ret.unite(map_container_with_value(PokemonInfo::TMMoves(num, gen), root ? QObject::tr("TM/HM") : QObject::tr("%1G TM/HM").arg(gen.num)))
              .unite(map_container_with_value(PokemonInfo::TutorMoves(num, gen), root ? QObject::tr("Tutor") : QObject::tr("%1G Tutor").arg(gen.num)))
              .unite(map_container_with_value(PokemonInfo::LevelMoves(num, gen), root ? QObject::tr("Level") : QObject::tr("%1G Level").arg(gen.num)))
              .unite(map_container_with_value(PokemonInfo::PreEvoMoves(num, gen), root ? QObject::tr("Pre Evo") :QObject:: tr("%1G Pre Evo").arg(gen.num)))
              .unite(map_container_with_value(PokemonInfo::EggMoves(num, gen), root ? QObject::tr("Breeding") : QObject::tr("%1G Breeding").arg(gen.num)))
              .unite((gen.num == 5 ? map_container_with_value(PokemonInfo::dreamWorldMoves(num, gen), QObject::tr("Dream World")) : QHash<int, QString>()))
              .unite(map_container_with_value(PokemonInfo::SpecialMoves(num, gen), root ? QObject::tr("Special", "Learning") : QObject::tr("%1G Special").arg(gen.num)));
}

void PokeMovesModel::loadData()
{
    QHash<int, QString> sets = getMoves(id, gen);
    storage.clear();

    foreach(int key, sets.uniqueKeys()) {
        storage.insert(MoveInfo::Name(key), QPair<int, QString>(key, sets[key]));
    }

    names = storage.keys();
}

int PokeMovesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return names.size();
}

int PokeMovesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return LastColumn;
}

QVariant PokeMovesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (role) {
    case Qt::DisplayRole:
        if (section == Type) {
            return tr("Type");
        } else if (section == Name) {
            return tr("Name", "AttackName");
        } else if (section == Learning) {
            return tr ("Learning");
        } else if (section == PP) {
            return tr("PP");
        } else if (section == Pow) {
            return tr("Pow");
        } else if (section == Acc) {
            return tr("Acc");
        } else if (section == Category) {
            return tr("Category");
        }
    case Qt::ToolTipRole:
        if (section == Type) {
            return tr("The type of the attack");
        } else if (section == Name) {
            return tr("The name of the attack");
        } else if (section == Learning) {
            return tr ("The way the pokemon learns the attack. There may be several ways a pokemon learns an attack, "
                       "in which case only the most recent/easy way is displayed.");
        } else if (section == PP) {
            return tr("The total number of PP for the attack, including PP ups.");
        } else if (section == Pow) {
            return tr("The power of the attack. ??? indicates varying power, -- indicates it never deals direct damage.");
        } else if (section == Acc) {
            return tr("The accuracy of the attack, in percentage. -- indicates it never misses.");
        } else if (section == Category) {
            return tr("The type of damage the attack deals, or Other if it doesn't deal direct damage");
        }
    };
    return QVariant();
}

QVariant PokeMovesModel::data(const QModelIndex &index, int role) const
{
    int movenum = storage[names[index.row()]].first;
    int section = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (section == Type) {
            return TypeInfo::Name(MoveInfo::Type(movenum, gen));
        } else if (section == Name) {
            return names[index.row()];
        } else if (section == Learning) {
            return storage[names[index.row()]].second;
        } else if (section == PP) {
            return MoveInfo::PP(movenum, gen);
        } else if (section == Pow) {
            int power = MoveInfo::Power(movenum, gen);
            if (power > 1) return power; 
            return MoveInfo::PowerS(movenum, gen);
        } else if (section == Acc) {
            int acc = MoveInfo::Acc(movenum, gen);
            if (acc <= 100) return acc;
            return MoveInfo::AccS(movenum, gen);
        } else if (section == Category) {
            return CategoryInfo::Name(MoveInfo::Category(movenum, gen));
        }
    case Qt::DecorationRole:
        if (section == Type) {
            return Theme::TypePicture(MoveInfo::Type(movenum, gen));
        }
        return QVariant();
    case Qt::TextColorRole:
        if (section == Category) {
            return Theme::CategoryColor(MoveInfo::Category(movenum, gen));
        } else if (section == Type) {
            return Theme::TypeColor(MoveInfo::Category(movenum, gen));
        }
        return QVariant();
    case Qt::SizeHintRole:
        if (section == Type) {
            return Theme::TypePicture(Type::Normal).size();
        }
        return QVariant();
    case CustomModel::MovenumRole:
        return movenum;
    case Qt::ToolTipRole:
        return MoveInfo::Description(movenum, gen);
    }

    return QVariant();
}

void PokeMovesModel::setPokemon(const Pokemon::uniqueId &id, Pokemon::gen gen)
{
    if (id == this->id && gen == this->gen) {
        return;
    }

    emit layoutAboutToBeChanged();
    this->id = id;
    this->gen = gen;

    loadData();
    emit layoutChanged();
}
