import QtQuick 2.2
import "../js/units.js" as U
Rectangle {
    id: root

    property alias text: buttonText.text
    property real padding: U.dp(0.1)

    signal triggered();

    width: buttonText.width + padding * 2
    height: buttonText.height + padding * 2
    radius: U.dp(0.1)
    border.color: "black"
    border.width: 2
    //color: Theme.buttonColor //TODO Theme
    Text {
        id: buttonText
        anchors.centerIn: parent
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.triggered();
    }
}
