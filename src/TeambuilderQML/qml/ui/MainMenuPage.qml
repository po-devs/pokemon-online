import QtQuick 2.0
import "../components"

Rectangle {
    id: root
    signal buildTeamClicked;

    anchors.fill: parent
    Column {
        width: parent.width
        Text {
            text: "Pokemon battle"
        }

        Button {
            text: "Start game"
            onTriggered: {
                root.buildTeamClicked();
            }
        }
    }
}
