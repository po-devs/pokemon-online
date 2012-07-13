import QtQuick 1.1
import "../" 1.0

Item {
    property FieldPokemon defender;
    property int attack;
    property FieldPokemon attacker;
    property variant params;
    property variant extras;

    function start() {
        animation.running = true;
    }

    signal finished();
}
