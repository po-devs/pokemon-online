import QtQuick 1.0
import "../" 1.0

Move {
    id: main;

    property int xt: defender.x+40;
    property int yt: defender.y+40;

    property int x0: attacker.x+60;
    property int y0: attacker.y+50;

    property int dirx: woof.back? 1 : -1;
    property int diry: -dirx;

    property int xt2: xt + 120*dirx;
    property int yt2: yt + 120*diry;

    Image {
        id: bone;

        source: "image://pokeinfo/item/200" //200 is thick club
        parent: main.parent.parent
        z: attacker.infront(1);
        x: x0;
        y: y0;
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
                        NumberAnimation { target: bone; property: "x"; to: xt; duration: 500; }
                        NumberAnimation { target: bone; property: "y"; to: yt; duration: 500; }
                        NumberAnimation { target: bone; property: "z"; to: defender.infront(1); duration: 500; }
                    }
                }
                SequentialAnimation {
                    loops: 2
                    NumberAnimation { target: bone; property: "rotation"; from: 0; to: 360; duration: 250 }
                }
            }

            ScriptAction { script: finished(); }
    }

    function start() {
        for (var i in extras) {
            console.log(i + ": " + extras[i]);
        }

        if ((extras.currentHit || 0) == 0) {
            finished();
        } else {
            animation.running = true;
        }
    }
}
