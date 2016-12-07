import QtQuick 2.0
import "../js/units.js" as U
import "../components"
import PokemonOnlineQml 1.0
Rectangle {
    signal buildTeamClicked;
    ServerChoiceModel {
        id: serverChoiceModel
    }
    ListView {
        id: serverListView
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: teambuilderButton.top
        }
        model: serverChoiceModel
        delegate: Text {
            text: name
        }
    }
    Button {
        id: teambuilderButton
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        text: "Build Team"
        onTriggered: buildTeamClicked()
    }
}
