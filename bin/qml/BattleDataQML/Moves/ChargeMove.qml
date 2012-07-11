import QtQuick 1.1
import "../" 1.0

Move {
    id: main;

    property int xt: defender.x-attacker.x;
    property int yt: attacker.y-defender.y;

    property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int y0: attacker.pokeSprite.anchors.bottomMargin;

    SequentialAnimation  {
        id: animation;
            ParallelAnimation {
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: params.attack_time; easing.type: params.easing_in_x; }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: params.attack_time; easing.type: params.easing_in_y; }
            }
            ParallelAnimation {
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: params.return_time; easing.type: params.easing_out_x; }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: params.return_time; easing.type: params.easing_out_y; }
            }

            ScriptAction { script: finished(); }
    }

    function start() {
            animation.running = true;
    }
}
