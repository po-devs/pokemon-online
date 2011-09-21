import QtQuick 1.0
import Qt.labs.particles 1.0

Item {
    id: main

    Rectangle {
        id: overlay;
        parent: main.parent
        anchors.fill: main.parent;
        opacity: 0.0;
        color: "#deac00";
        z: 10;
    }

    Particles {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: main.parent.height;
        id: particles
        source: "../../images/sand.png"
        opacity: 0;

        lifeSpan: 1000
        angle: 165
        angleDeviation: 20
        velocity: 400
        count:0;
        emissionRate: 300;
        fadeInDuration: 50;

        velocityDeviation: 10
        ParticleMotionWander {
            pace: 100
            xvariance: 80
        }
    }

    SequentialAnimation {
        id: animation;
        running: false;

        ScriptAction {
            script: {particles.opacity = 1; battle.scene.pause();}
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
            duration: 300;
        }
        ScriptAction {
            script: particles.count = 2000;
        }
        PauseAnimation {
            duration: 2000;
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
