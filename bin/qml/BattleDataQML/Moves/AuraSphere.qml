import QtQuick 1.1
import Qt.labs.particles 1.0
import "../" 1.0

Move {
    id: main;

    property int xt: defender.x + (defender.pokeSprite.width-aura.width)/2;
    property int yt: defender.y + defender.pokeSprite.height - defender.pokeSprite.height*defender.pokeSprite.scale*0.4
                     - aura.height*defender.pokeSprite.scale/2;
    property int z0: attacker.infront(1)+attacker.pokeSprite.z;

    property int x0: attacker.x + attacker.pokeSprite.x + (attacker.pokeSprite.width-aura.width)/2
    property int y0: attacker.y + attacker.pokeSprite.y + attacker.pokeSprite.height - attacker.pokeSprite.height*attacker.pokeSprite.scale*0.4
                     - aura.height*attacker.pokeSprite.scale*0.4;

    Particles {
        parent: defender;
        x: defender.pokeSprite.x + defender.pokeSprite.width/2 - width/2
        y: defender.pokeSprite.y+defender.pokeSprite.height -defender.pokeSprite.height*0.4*scale;
        z: defender.pokeSprite.z+2;
        width: 2
        height: 2
        scale: defender.pokeSprite.scale;
        id: particles
        property int n: 1;
        source: "../../images/lightparticle" + n +".png"

        lifeSpan: 800
        angle: 0
        angleDeviation: 360
        velocity: 40
        count: 0;
        emissionRate: 80;
        opacity: 1;

        velocityDeviation: 10
    }

    property int oxt: defender.x-attacker.x;
    property int oyt: attacker.y-defender.y;
    property int ozt: defender.z-attacker.z;

    property int ox0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int oy0: attacker.pokeSprite.anchors.bottomMargin;
    property int oz0: attacker.pokeSprite.z;

    property real pokeAdvance: 0.1;
    property int throwDuration: 1500;

    Image {
        id: aura;

        source: "../../images/aurasphere.png"
        parent: main.parent.parent
        z: z0;
        x: x0;
        y: y0;
        scale: calculateScale(z)*0.8;
        //scale: calculateScale(z)*0.4;
        //width: implicitWidth*1.3
        //rotation: -30
    }

    SequentialAnimation  {
        id: animation;

        ScriptAction {
            /* Bind the values so they don't change */
            script: {
                x0 = x0;
                y0 = y0;
                z0 = z0;
            }
        }

        ParallelAnimation {
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: oxt*pokeAdvance; duration: throwDuration*pokeAdvance*2; }
            NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: oyt*pokeAdvance; duration: throwDuration*pokeAdvance*2;}
            NumberAnimation { target: attacker.pokeSprite; property: "z"; to: ozt*pokeAdvance; duration: throwDuration*pokeAdvance;}
            NumberAnimation { target: aura; property: "x"; from: x0; to: xt; duration: throwDuration; }
            NumberAnimation { target: aura; property: "y"; from: y0; to: yt; duration: throwDuration; }
            NumberAnimation { target: aura; property: "z"; from: z0; to: defender.infront(1); duration: throwDuration; }
            NumberAnimation { target: aura; property: "rotation"; from: 0; to: 360*3; duration: throwDuration }
            SequentialAnimation {
                PauseAnimation { duration: throwDuration*pokeAdvance+100 }
                ParallelAnimation {
                    NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: ox0; duration: throwDuration*pokeAdvance*2; easing: Easing.InOutQuad}
                    NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: oy0; duration: throwDuration*pokeAdvance*2; easing: Easing.InOutQuad}
                    NumberAnimation { target: attacker.pokeSprite; property: "z"; to: oz0; duration: throwDuration*pokeAdvance*2; easing: Easing.InOutQuad}
                }
            }
        }
        ScriptAction {
            script: particles.count = 120;
        }
        ParallelAnimation {
            NumberAnimation {target: aura; property: "opacity"; to: 0; duration: 500;}
            SequentialAnimation {
                loops: 10
                /* Random color */
                ScriptAction {script: particles.n = Math.random()*9 + 1}
                PauseAnimation {duration: 100}
            }
        }

        ScriptAction { script: finished(); }
    }
}
