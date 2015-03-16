import QtQuick 2.0
import "../components"
import PokemonOnlineQml 1.0

Rectangle {
    signal goBack;
    anchors.fill: parent
    AnalyzerAccess {
        id: analyserAccess
        Component.onCompleted: connectTo("188.165.244.152", 5080)
    }

    Column {
        width: parent.width
        Text {
            text: "Online players"
        }
        Button {
            text: "Back"
            onTriggered: goBack();
        }
    }
}
