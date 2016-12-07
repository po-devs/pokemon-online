import QtQuick 2.0
import "../js/units.js" as U
Item {
    id: root

    property string playerName

    signal decline;

    anchors.fill: parent
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.05
    }
    Rectangle {
        width: U.dp(4);
        height: U.dp(3)
        border {
            color: "black"
            width: 2
        }
        radius: U.dp(0.1)
        anchors.centerIn: parent
        Column {
            width: parent.width
            Text {
                text: "Player " + playerName + " challenges you to a game"
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
            }
            Row {
                height: U.dp(0.5)
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    text: "Accept"
                }
                Button {
                    text: "Decline"
                    onTriggered: {
                        decline();
                        root.destroy();
                    }
                }
            }
        }
    }
}

