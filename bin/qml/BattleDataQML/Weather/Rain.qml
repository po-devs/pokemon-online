import QtQuick 1.0
import Qt.labs.particles 1.0

Item {
    id: main

    Rectangle {
        id: overlay;
        parent: main.parent
        anchors.fill: main.parent;
        opacity: 0.0;
        color: "#5a69b5";
        z: -80;
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

        velocityDeviation: 10
        ParticleMotionLinear {
        }
    }

    SequentialAnimation {
        id: animation;
        running: false;

        ScriptAction {
            script: {battle.scene.pause();}
        }
        NumberAnimation {
            target: overlay; duration: 250;
            property: "opacity"; to: 0.8;
        }
        PauseAnimation {
            duration: 300;
        }
        ScriptAction {
            script: particles.count = 500;
        }
        PauseAnimation {
            duration: 1800;
        }
        ScriptAction {
            script: particles.count = 0;
        }
        PauseAnimation {
            duration: 200;
        }
        NumberAnimation {
            target: overlay; duration: 250;
            property: "opacity"; to: 0.0;
        }
        ScriptAction {
            script: {battle.scene.unpause();}
        }
    }

    function start() {
        animation.running = true;
    }
}
