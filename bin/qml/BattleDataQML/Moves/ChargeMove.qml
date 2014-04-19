import QtQuick 2.0
import QtQuick.Particles 2.0
import "../" 1.0

/* ChargeMove contains logic for many changelike attacks.
 * Params:
 *  attack_time: milliseconds for offensive part
 *  return_time: milliseconds for return part
 *  rolls: number of rolls sprite does when attacking
 *  easing_in_x, easing_in_y: QML easier for charge animation on x and y
 *  easing_out_x, easing_out_y: QML easier for return animation on x and y
 *  effect: any image source for particles that are emitted while charging
 */

Move {
    id: main;

    property int xt: defender.x-attacker.x;
    property int yt: attacker.y-defender.y;
    property int zt: defender.z-attacker.z;

    property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
    property int y0: attacker.pokeSprite.anchors.bottomMargin;
    property int z0: attacker.pokeSprite.z;

    ParticleSystem {
        id: particles;

        x: attacker.pokeSprite.x + attacker.pokeSprite.width/2 - width/2
        y: attacker.pokeSprite.y + attacker.pokeSprite.height-35
        width: 2
        height: 2

        ImageParticle {
            id: imgparticle
            source: ""
        }

        Emitter {
            lifeSpan: 1400;
            lifeSpanVariation: 100;

            velocity: AngleDirection {
                angle: 270;
                angleVariation: 50
                magnitude: 100
                magnitudeVariation: 20
            }

            maximumEmitted: parent.count;
            enabled: parent.count > 0;

            acceleration: AngleDirection {
                angle: 90;
                magnitude: 100;
            }
        }

        property int count: 0

        z: attacker.pokeSprite.z + 10
    }

    property int origin: 0

    SequentialAnimation  {
        id: animation;
            ScriptAction { script: {
                    if (params.effect) {
                        imgparticle.source = params.effect
                        particles.count = 100;
                    }
                    if (params.rolls) {
                        origin = attacker.pokeSprite.transformOrigin
                        attacker.pokeSprite.transformOrigin = Item.Center
                    }
                }
            }
            ParallelAnimation {
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: params.attack_time; easing.type: params.easing_in_x; }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: params.attack_time; easing.type: params.easing_in_y; }
                NumberAnimation { target: attacker.pokeSprite; property: "z"; to: zt; duration: params.attack_time; easing.type: params.easing_in_y; }
                NumberAnimation { target: attacker.pokeSprite; property: "rotation"; from: 0.0; to: params.rolls * 360.0 * (attacker.back?1:-1) ; duration: params.attack_time}
            }
            ScriptAction { script: {
                    if (params.rolls) {
                        attacker.pokeSprite.transformOrigin = origin;
                    }
                    particles.count=0;
                }
            }
            ParallelAnimation {
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: params.return_time; easing.type: params.easing_out_x; }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: params.return_time; easing.type: params.easing_out_y; }
                NumberAnimation { target: attacker.pokeSprite; property: "z"; to: z0; duration: params.return_time; easing.type: params.easing_out_y; }
            }

            ScriptAction { script: finished(); }
    }

    function start() {
            animation.running = true;
    }
}
