import QtQuick 1.1
import Qt.labs.particles 1.0
import "../" 1.0

/* ChargeMove contains logic for many changelike attacks.
 * Params:
 *  attack_time: milliseconds for offensive part
 *  rolls: number of rolls sprite does when attacking
 *  easing_in_x, easing_in_y: QML easier for charge animation on x and y
 *  easing_out_x, easing_out_y: QML easier for return animation on x and y
 *  image: any particles that are emitted while charging
 */

Move {
    id: main;

    property int xt: defender.x-attacker.x;
    property int yt: attacker.y-defender.y;

    property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int y0: attacker.pokeSprite.anchors.bottomMargin;

    Particles {
        id: particles;

        x: attacker.pokeSprite.x + attacker.pokeSprite.width/2 - width/2
        y: attacker.pokeSprite.y + attacker.pokeSprite.height-35
        width: 2
        height: 2
        source: ""

        lifeSpan: 1400
        lifeSpanDeviation: 100
        angle: 270
        angleDeviation: 50
        velocity: 100
        count: 0
        emissionRate: 100

        velocityDeviation: 20
        z: attacker.pokeSprite.z + 10

        ParticleMotionGravity {
            yattractor: 1000
            acceleration: 100
        }
    }

    SequentialAnimation  {
        id: animation;
            ScriptAction { script: {
                    if (params.effect) {
                        particles.source = params.effect
                        particles.count = 100;
                    }
                }
            }
            ParallelAnimation {
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: params.attack_time; easing.type: params.easing_in_x; }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: params.attack_time; easing.type: params.easing_in_y; }
                /* For some reason Rotation is not triggering */
                RotationAnimation { target: attacker.pokeSprite; property: "rotation"; from: 0.0; to: params.rolls * 360.0; duration: params.attack_time; direction: RotationAnimation.Clockwise; }
            }
            ScriptAction { script: particles.count=0; }
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
