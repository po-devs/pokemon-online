import QtQuick 1.1
import "../" 1.0

Move {
    id: main;

    /*
     * The main animation of Rapid Spin
     */
    SequentialAnimation  {
        id: animation;
        // Shake first 10 times
        SequentialAnimation  {
            loops: 10;
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: 55; duration: 50 }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: 0; duration: 50 }
        }
        // Finally center it
        NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: 0; duration: 100 }
        // Delete the Move object
        ScriptAction { script: finished(); }
    }
}
