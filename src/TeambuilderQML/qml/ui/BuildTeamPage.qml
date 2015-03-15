import QtQuick 2.0
import PokemonOnlineQml 1.0
import "../components"
Rectangle {
    signal goBack;

    TeamHolder {
        id: teamHolder
    }

    PokeTableModel {
        id: pokeTableModel;
        //hold a list of all pokemons
    }

    Column {
        Button {
            text: "Back to server list"
            onTriggered: goBack();
        }
    }
    Flow {
        width: parent.width
        Repeater {
            model: teamHolder.team;
            delegate: Text {
                text: "a pokemon"
            }
        }
    }
}
