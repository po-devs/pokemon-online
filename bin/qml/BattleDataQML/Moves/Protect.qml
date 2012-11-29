// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Move {
    id: main

    property int xt: attacker.pokeSprite.anchors.horizontalCenterOffset+20*(1-2*woof.back);
    property int yt: attacker.pokeSprite.anchors.bottomMargin+20*(1-2*woof.back);
    property int zt: attacker.pokeSprite.z + (woof.back ? 10 : -10);

    property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int y0: attacker.pokeSprite.anchors.bottomMargin;
    property int z0: attacker.pokeSprite.z;

    property real completion: 0

    Rectangle {
        id: screen
        parent: woof

        width: 100*completion
        height: 100*completion
        color: params.color
        border.color: Qt.darker(color)
        radius: 6
        border.width: 2
        opacity: 0.7

        z: woof.back ? -8*completion : 8*completion;
        x: woof.back ? woof.width*0.3*completion : woof.width - width - woof.width*0.3*completion
        y: woof.back ? woof.height-height*0.75-0.3*woof.height*completion : 0.3*woof.height*completion
        scale: calculateScale(woof.z+z)
    }

    SequentialAnimation  {
        id: animation;

        ParallelAnimation {
            NumberAnimation { target: main; property: "completion"; to: 1; duration: 1700; easing.type: Easing.OutElastic; easing.period: 0.25 }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: 200 }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: 200 }
            NumberAnimation { target: attacker.pokeSprite; property: "z"; to: zt; duration: 200 }
        }
        ParallelAnimation {
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: 50 }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: 50 }
            NumberAnimation { target: attacker.pokeSprite; property: "z"; to: z0; duration: 50 }
        }
        // Delete the Move object
        ScriptAction { script: {
                finished();
            }
        }
    }

    function start() {
        animation.running = true;
    }
}
