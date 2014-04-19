import QtQuick 2.0
import QtQuick.Particles 2.0

Image {
    id: main
    opacity: 0;
    source: "../../images/sun.png"
    anchors.fill: parent;
    z: 200;

    ParticleSystem {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: main.parent.height;
        id: particles
        opacity: 0;

        ImageParticle {
            anchors.fill: parent
            source: "../../images/lightparticle.png"
        }

        Emitter {
            anchors.fill: parent
            lifeSpan: 800
            emitRate: 50;
            maximumEmitted: parent.count
            enabled: parent.count > 0

            velocity: AngleDirection {
                angle: 360
                magnitude: 20
                magnitudeVariation: 10
            }
        }

        property int count: 0
    }

    SequentialAnimation {
        running: false;
        id: animation;
        ScriptAction {
            script: {battle.scene.pause();}
        }
        ParallelAnimation {
            NumberAnimation { target: main; property: "opacity"; to: 1.0; duration: 2000; easing.type: Easing.OutBounce }
            SequentialAnimation {
                PauseAnimation {duration: 1000}
                ScriptAction { script: particles.count = 60;}
                NumberAnimation {target: particles; property: "opacity"; to: 2.0; duration: 250}
                PauseAnimation {duration: 1500}
                ScriptAction { script: particles.count = 0;}
            }
        }
        PauseAnimation { duration: 1000 }
        NumberAnimation { targets: [main, particles]; property: "opacity"; to: 0; duration: 350 }
        ScriptAction {
            script: {particles.count = 0; battle.scene.unpause();}
        }
    }

    function start() {
        animation.running = true;
    }
}
