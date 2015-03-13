import QtQuick 2.0
import "../components"
Rectangle {
    signal goBack;
    Button {
        text: "Back to server list"
        onTriggered: goBack();
    }
}
