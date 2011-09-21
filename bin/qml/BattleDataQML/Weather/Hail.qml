import QtQuick 1.0
import Qt.labs.particles 1.0

Item {
    id: main

    Rectangle {
        id: overlay;
        parent: main.parent
        anchors.fill: main.parent;
        opacity: 0.0;
        color: "black";
        z: -80;
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
            script: particles.burst(30, 60);
        }
        PauseAnimation {
            duration: 1950;
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
