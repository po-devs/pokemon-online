import QtQuick 2.0
import QtQuick.Particles 2.0

Item {
    id: main
    z: 200;

    Rectangle {
        id: overlay;
        parent: main.parent
        anchors.fill: main.parent;
        opacity: 0.0;
        color: "black";
    }

    ParticleSystem {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: 250;
        id: particles

        ImageParticle {
            source: "../../images/hailstone.png"
        }

        Emitter {
            lifeSpan: 1600
            velocity: AngleDirection {
                angle: 70; angleVariation: 36;
                magnitude: 120; magnitudeVariation: 10
            }
            maximumEmitted: parent.count
            enabled: parent.count > 0
            emitRate: 60

            anchors.fill: parent
        }

        Wander {
            xVariance: 20
            pace: 100
        }

        opacity: 0;
    }

    SequentialAnimation {
        id: animation;
        running: false;

        ScriptAction {
            script: {battle.scene.pause();particles.count=500;}
        }
        ParallelAnimation {
            NumberAnimation {
                target: particles; duration: 250;
                property: "opacity"; to: 1;
            }
            NumberAnimation {
                target: overlay; duration: 250;
                property: "opacity"; to: 0.5;
            }
        }
        PauseAnimation {
            duration: 2300;
        }
        NumberAnimation {
            targets: [overlay, particles]; duration: 250;
            property: "opacity"; to: 0.0;
        }
        ScriptAction {
            script: {particles.count = 0; battle.scene.unpause();}
        }
    }

    function start() {
        animation.running = true;
    }
}
