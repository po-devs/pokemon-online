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
        color: "#deac00";
    }

    ParticleSystem {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: main.parent.height;
        id: particles
        opacity: 0;

        ImageParticle {
            source: "../../images/sand.png"
            anchors.fill: parent
        }

        Emitter {
            anchors.fill: parent
            maximumEmitted: parent.count
            enabled: parent.count > 0
            lifeSpan: 1000;
            velocity: AngleDirection {
                angle: 165
                angleVariation: 20
                magnitude: 400
                magnitudeVariation: 10
            }
        }

        Wander {
            pace: 100
            xVariance: 80
        }

        property int count:0;
    }

    SequentialAnimation {
        id: animation;
        running: false;
        ScriptAction {
            script: {battle.scene.pause(); particles.count = 2000;}
        }
        ParallelAnimation {
            NumberAnimation {
                target: particles; duration: 250;
                property: "opacity"; to: 1.0;
            }
            NumberAnimation {
                target: overlay; duration: 250;
                property: "opacity"; to: 0.6;
            }
        }
        PauseAnimation {
            duration: 2300;
        }
        ParallelAnimation {
            NumberAnimation {
                target: overlay; duration: 250;
                property: "opacity"; to: 0.0;
            }
            NumberAnimation {
                target: particles; duration: 250;
                property: "opacity"; to: 0.0;
            }
        }
        ScriptAction {
            script: {particles.count = 0; battle.scene.unpause();}
        }
    }

    function start() {
        animation.running = true;
    }
}
