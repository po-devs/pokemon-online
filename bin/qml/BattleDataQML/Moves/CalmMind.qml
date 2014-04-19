import QtQuick 2.0
import QtQuick.Particles 2.0
import "../" 1.0

Move {
    id: main;

    ParticleSystem {
        parent: defender;
        x: attacker.pokeSprite.x + attacker.pokeSprite.width/2 - width/2
        y: attacker.pokeSprite.y+attacker.pokeSprite.height -attacker.pokeSprite.height/2*scale;
        z: attacker.pokeSprite.z+2;
        width: 2
        height: 2
        scale: attacker.pokeSprite.scale;
        id: particles

        ImageParticle {
            source: "../../images/lightparticle.png"
        }

        Emitter {
            lifeSpan: 800
            emitRate: 80;
            maximumEmitted: parent.count
            enabled: parent.count > 0

            velocity: AngleDirection {
                magnitude: 40; magnitudeVariation: 10;
                angle: 0; angleVariation: 360;
            }
        }


        property int count: 0;
        opacity: 1;
    }
    Image {
        id: cm
        source: "../../images/calmmind.png"
        parent: attacker;
        z: attacker.pokeSprite.z+1;
        anchors.horizontalCenter: particles.horizontalCenter
        anchors.verticalCenter: particles.verticalCenter
        scale: attacker.pokeSprite.scale*(1-2*completion/3)
        transformOrigin: Item.Center
        smooth: false
        opacity: (1-completion)*Math.sqrt(Math.sqrt(completion))
        property real completion: 0;
    }

    /*
     * The main animation of Calm mind
     */
    SequentialAnimation  {
        id: animation;
        //ScriptAction {script: particles.count = 60; }
        ParallelAnimation {
            SequentialAnimation {
                PauseAnimation { duration: 300 }
                ScriptAction {script: particles.count=120;}
                PauseAnimation {duration: 1600}
                ScriptAction {script: particles.count = 0;}
            }

            SequentialAnimation {
                loops: 2
                NumberAnimation { target: cm; property: "completion"; duration: 1200;
                    from: 0; to: 1; easing.type: Easing.InQuad }
                ScriptAction{script: cm.completion = 1;}
                PauseAnimation{duration: 100;}
            }
        }

        // Delete the Move object
        ScriptAction { script: finished(); }
    }
}
