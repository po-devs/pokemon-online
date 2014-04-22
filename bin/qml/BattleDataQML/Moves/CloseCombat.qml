import QtQuick 2.0
import QtQuick.Particles 2.0
import "../" 1.0

Move {
        id: main;

        property int x1: attacker.pokeSprite.anchors.horizontalCenterOffset+10*(1-2*fieldPoke.back);
        property int x2: attacker.pokeSprite.anchors.horizontalCenterOffset-10*(1-2*fieldPoke.back);

        property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;

        /*
         * The main animation of Close combat
         * Lucky punch is used as a (placeholder) image
         */
        ParticleSystem {
            parent: defender.pokeSprite.parent
            z: defender.infront(5)
            width: defender.width;
            height: defender.height;

            ImageParticle {
                source: "../../images/fist.png"
            }

            Emitter {
                lifeSpan: 200
                maximumEmitted: 10
                velocity: AngleDirection {
                    magnitude: 0; magnitudeVariation: 5;
                    angle: 0; angleVariation: 180
                }
            }
            scale: calculateScale(defender.z)

            transformOrigin: Item.Bottom
        }        

        SequentialAnimation  {
                id: animation;
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x2; duration: 150 }
                SequentialAnimation  {
                    loops: 3;
                    NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x1; duration: 300 }
                    NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x2; duration: 300 }
                }
                NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: 150 }
                // Delete the Move object
                ScriptAction { script: finished(); }
        }
}
