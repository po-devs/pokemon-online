import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    id: item;
    property int player
    property TeamData team: battle.data.team(player)

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: 32
        cellHeight: 32
        property bool loaded: false;

        model : ListModel {
        }

        delegate: Pokemon {

        }

        Component.onCompleted: {
            for (var i = 0; i < 6; i++) {
                model.insert(i, {"pokemon": team.poke(i)});
            }
            loaded = true;
        }
    }

    Connections {
        target: team
        onPokemonsSwapped: {
            var min = Math.min(slot1, slot2);
            var max = Math.max(slot1, slot2);
            grid.model.move(min, max, 1);
            grid.model.move(max-1, min, 1);
        }
    }
}
