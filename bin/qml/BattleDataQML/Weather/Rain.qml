import QtQuick 1.1
import Qt.labs.particles 1.0

Item {
    id: main
    z: 300;

    Rectangle {
        id: overlay;
        parent: main.parent
        anchors.fill: main.parent;
        opacity: 0.0;
        color: "#5a69b5";
    }

    Particles {
        parent: main.parent;
        y: 0;
        x: -100;
        width: main.parent.width+100
        height: 1;
        id: particles
        source: "../../images/raindrop.png"

        lifeSpan: 1000
        angle: 75
        angleDeviation: 10
        velocity: 500
        count:0;
        emissionRate: 80;
        opacity: 0;

        velocityDeviation: 10
        ParticleMotionLinear {
        }

        z: 20;
    }

    SequentialAnimation {
        id: animation;
        running: false;

        ScriptAction {
            script: {battle.scene.pause();particles.count = 500}
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
            duration: 2000;
        }
        ScriptAction {
            script: particles.count = 0;
        }
        PauseAnimation {
            duration: 200;
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
