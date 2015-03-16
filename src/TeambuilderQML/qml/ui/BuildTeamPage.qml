import QtQuick 2.0
import PokemonOnlineQml 1.0
import "../components"
Rectangle {
    signal goBack;
    signal goFindGame;

    anchors.fill: parent

    TeamHolder {
        id: teamHolder
    }

    PokeTableModel {
        id: pokeTableModel;
        //hold a list of all pokemons
    }

    Column {
        width: parent.width
        Text {
            text: "Build team"
        }

        Button {
            text: "Back to server list"
            onTriggered: goBack();
        }

        Button {
            text: "Find game"
            onTriggered: goFindGame();
        }
    }
}
