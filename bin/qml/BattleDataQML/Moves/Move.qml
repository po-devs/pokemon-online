import QtQuick 1.0
import "../" 1.0

Item {
    property FieldPokemon defender;
    property int attack;
    property FieldPokemon attacker;
    property variant params;
    property variant extras;

    signal finished();
}
