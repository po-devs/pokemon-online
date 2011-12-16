import QtQuick 1.0
import "../" 1.0

Item {
    property FieldPokemon target;
    property int attack;
    property FieldPokemon attacker;
    property variant params;

    signal finished();
}
