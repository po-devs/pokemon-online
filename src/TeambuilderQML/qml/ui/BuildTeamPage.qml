import QtQuick 2.0
import PokemonOnlineQml 1.0
import "../components"
import "../js/units.js" as U
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

        Rectangle {
            width: U.dp(2)
            height: U.dp(0.3)
            border {
                color: "black"
                width: 2
            }

            TextInput {
                id: nameInput
                anchors.fill: parent
                onTextChanged: analyserAccess.setPlayerName(text)
            }
        }

        Button {
            text: "Find game"
            onTriggered: goFindGame();
        }
    }
}
