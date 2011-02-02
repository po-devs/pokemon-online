#include "theme.h"
#include "../PokemonInfo/pokemoninfo.h"

QList<QColor> EmptyList() 
{
    QList<QColor> list;
    return list;
}

QList<QColor> Theme::m_TColors = EmptyList() << "#a8a878" << "#c03028" << "#a890f0" << "#a040a0" << "#e0c068" << "#b8a038" << "#a8b820" << "#705898" << "#b8b8d0" << "#f08030" << "#6890f0" << "#78c850" << "#f8d030" << "#f85888" << "#98d8d8" << "#7038f8" << "#705848" << "#68a090" << "";
QList<QColor> Theme::m_CColors = EmptyList() << "#5811b1" << "#399bcd" << "#0474bb" << "#f8760d" << "#a00c9e" << "#0d762b" << "#5f4c00" << "#9a4f6d" << "#d0990f" << "#1b1390" << "#028678" << "#0324b1";
QList<QColor> Theme::m_ChatColors = EmptyList() << "#68a090" << "#c03028" << "#f85888";

QColor Theme::TypeColor(int typenum)
{
    return m_TColors[typenum];
}

QColor Theme::CategoryColor(int typenum)
{
    return m_CColors[typenum];
}

QColor Theme::ChatColor(int num)
{
    return m_ChatColors[num % m_ChatColors.size()];
}

QColor Theme::StatusColor(int status)
{
    switch (status) {
    case Pokemon::Koed: return "#171b1a";
    case Pokemon::Fine: return TypeColor(Pokemon::Normal);
    case Pokemon::Paralysed: return TypeColor(Pokemon::Electric);
    case Pokemon::Burnt: return TypeColor(Pokemon::Fire);
    case Pokemon::Frozen: return TypeColor(Pokemon::Ice);
    case Pokemon::Asleep: return TypeColor(Pokemon::Psychic);
    case Pokemon::Poisoned: return TypeColor(Pokemon::Poison);
    case Pokemon::Confused: return TypeColor(Pokemon::Ghost);
    default: return QColor();
    }
}