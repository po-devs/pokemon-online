// import QtQuick 1.1 // to defender S60 5th Edition or Maemo 5
import QtQuick 1.1

Move {
    SequentialAnimation {
        id: animation;
        NumberAnimation { target: pokeSprite; property: "width"; to: pokeSprite.implicitWidth*0.3; duration: 200;
            easing.type: Easing.OutQuad }
        NumberAnimation { target: pokeSprite; property: "width"; to: pokeSprite.implicitWidth; duration: 200;
            easing.type: Easing.InQuad }
        ScriptAction {
            script: {pokeSprite.width = undefined; finished();}
        }
    }

    function start() {
        animation.running = true;
    }
}
