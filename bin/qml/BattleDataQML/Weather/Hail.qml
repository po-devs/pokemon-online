import QtQuick 1.1
import Qt.labs.particles 1.0

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

    Particles {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: 250;
        id: particles
        source: "../../images/hailstone.png"

        lifeSpan: 1600
        angle: 70
        angleDeviation: 36
        velocity: 120
        count: 0;
        emissionRate: 60;
        opacity: 0;

        velocityDeviation: 10
        ParticleMotionWander {
            xvariance: 20
            pace: 100
        }
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
