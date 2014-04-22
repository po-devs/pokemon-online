import QtQuick 2.0
import "../" 1.0

Move {
    id: main;

    property int xt: defender.x+40;
    property int yt: defender.y+40;

    property int x0: attacker.x+60;
    property int y0: attacker.y+50;

    property int dirx: fieldPoke.back? 1 : -1;
    property int diry: -dirx;

    Image {
        id: bone;

        source: params.image
        parent: main.parent.parent
        z: attacker.infront(1);
        x: x0;
        y: y0;
        scale: calculateScale(z);
    }

    SequentialAnimation  {
            id: animation;

            ParallelAnimation {
                SequentialAnimation {
                    ScriptAction {
                        script: {
                                bone.x = x0;
                                bone.y = y0;
                                bone.z = attacker.infront(1);
                    }
                    }

                    ParallelAnimation {
                        NumberAnimation { target: bone; property: "x"; to: xt; duration: params.duration; easing.type: params.easing }
                        NumberAnimation { target: bone; property: "y"; to: yt; duration: params.duration; easing.type: params.easing }
                        NumberAnimation { target: bone; property: "z"; to: defender.infront(1); duration: params.duration; easing.type: params.easing }
                    }
                }
                NumberAnimation { target: bone; property: "rotation"; from: 0; to: 360*params.rotations; duration: params.duration }
            }

            ScriptAction { script: finished(); }
    }

    function start() {
        if ((extras.currentHit || 0) === 0) {
            finished();
        } else {
            animation.running = true;
        }
    }
}
