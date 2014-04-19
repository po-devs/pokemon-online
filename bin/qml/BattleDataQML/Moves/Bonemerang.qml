import QtQuick 2.0
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
        scale: calculateScale(z);
        x: x0;
        y: y0;
    }

    SequentialAnimation  {
            id: animation;
            running: false;

            ParallelAnimation {
                SequentialAnimation {
                    ParallelAnimation {
                        NumberAnimation { target: bone; property: "x"; to: xt; duration: 500; easing.type: Easing.OutQuad }
                        NumberAnimation { target: bone; property: "y"; to: yt; duration: 500; easing.type: Easing.InQuad }
                        NumberAnimation { target: bone; property: "z"; to: defender.infront(1); duration: 500; easing.type: Easing.InQuad }
                    }
                    ParallelAnimation {
                        NumberAnimation { target: bone; property: "x"; to: xt2; duration: 300; easing.type: Easing.InQuad }
                        NumberAnimation { target: bone; property: "y"; to: yt2; duration: 300; easing.type: Easing.OutQuad }
                        NumberAnimation { target: bone; property: "z"; to: defender.behind(50); duration: 300; easing.type: Easing.InQuad }
                    }
                    ParallelAnimation {
                        NumberAnimation { target: bone; property: "x"; to: xt; duration: 300; easing.type: Easing.InQuad }
                        NumberAnimation { target: bone; property: "y"; to: yt; duration: 300; easing.type: Easing.OutQuad }
                        NumberAnimation { target: bone; property: "z"; to: defender.behind(-1); duration: 300; easing.type: Easing.InQuad }
                    }
                    ParallelAnimation {
                        NumberAnimation { target: bone; property: "x"; to: x0; duration: 500; easing.type: Easing.OutQuad }
                        NumberAnimation { target: bone; property: "y"; to: y0; duration: 500; easing.type: Easing.InQuad }
                        NumberAnimation { target: bone; property: "z"; to: attacker.infront(1); duration: 500;}
                    }
                }
                SequentialAnimation {
                    loops: 8
                    NumberAnimation { target: bone; property: "rotation"; from: 0; to: 360; duration: 200 }
                }
            }

            ScriptAction { script: finished(); }
    }

    function start() {
        if ((extras.currentHit || 0) === 0) {
            animation.running = true;
        } else {
            finished();
        }
    }
}
