import QtQuick 1.1
import Qt.labs.particles 1.0

Move {
    id: main
    z: 300;

    Rectangle {
        id: overlay;
        parent: main.parent.parent
        anchors.fill: main.parent.parent
        opacity: 0.0;
        color: "#5a69b5";
    }

    SequentialAnimation {
        id: animation;
        running: false;
        NumberAnimation {
            target: overlay; duration: 250;
            property: "opacity"; to: 0.6;
        }
        PauseAnimation {
            duration: 1000
        }
        NumberAnimation {
            targets: overlay; duration: 250;
            property: "opacity"; to: 0.0;
        }
        ScriptAction {
            script: finished();
        }
    }
}
