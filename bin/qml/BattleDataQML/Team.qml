import QtQuick 1.1
import pokemononline.battlemanager.proxies 1.0

Item {
    id: item;
    property TeamData team
    z: 500;

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

            /* Todo: later, check for something else than singles */
            /* Should the logic of this really be in this file? */
            if (min == 0) {
                fieldPokemonChanged(min)
            }
        }
    }

    signal fieldPokemonChanged(int pokemon)
}
