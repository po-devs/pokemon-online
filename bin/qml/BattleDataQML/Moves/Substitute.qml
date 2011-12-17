// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Move {
    SequentialAnimation {
        id: animation;
        NumberAnimation { target: image; property: "width"; to: image.implicitWidth*0.3; duration: 200;
            easing.type: Easing.OutQuad }
        NumberAnimation { target: image; property: "width"; to: image.implicitWidth; duration: 200;
            easing.type: Easing.InQuad }
        ScriptAction {
            script: {image.width = undefined; finished();}
        }
    }

    function start() {
        animation.running = true;
    }
}
