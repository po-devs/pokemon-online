// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Move {
    id: main;

    property int xt: defender.x-attacker.x;
    property int yt: attacker.y-defender.y;
    property int zt: defender.z-attacker.z;

    property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int y0: attacker.pokeSprite.anchors.bottomMargin;
    property int z0: attacker.pokeSprite.z;

    Image {
        id: punch
        source: "../../images/fist.png"

        parent: defender;
        z: defender.infront(5);
        anchors.centerIn: defender;
        anchors.verticalCenterOffset: 10;
        opacity: 0;
    }

    SequentialAnimation {
        id: animation;

        ParallelAnimation {
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt/5; duration: 90; easing.type: Easing.Linear; }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt/5; duration: 90; easing.type: Easing.Linear; }
            NumberAnimation { target: attacker.pokeSprite; property: "z"; to: zt/5; duration: 90; easing.type: Easing.Linear; }
            SequentialAnimation { PauseAnimation { duration: 60 } NumberAnimation { target: punch; property: 'opacity'; to: 1; duration: 60; } }
        }

        PauseAnimation { duration: 200 }

        ScriptAction {
            script: punch.opacity = 0;
        }

        ParallelAnimation {
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: 350; easing.type: Easing.Linear; }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: 350; easing.type: Easing.Linear; }
            NumberAnimation { target: attacker.pokeSprite; property: "z"; to: z0; duration: 350; easing.type: Easing.Linear; }
        }

        ScriptAction { script: finished(); }
    }
}
