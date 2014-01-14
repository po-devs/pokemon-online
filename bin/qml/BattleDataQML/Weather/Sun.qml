import QtQuick 1.1
import Qt.labs.particles 1.0

Image {
    id: main
    opacity: 0;
    source: "../../images/sun.png"
    anchors.fill: parent;
    z: 200;

    Particles {
        parent: main.parent;
        y: 0;
        x: 0;
        width: main.parent.width
        height: main.parent.height;
        id: particles
        source: "../../images/lightparticle.png"
        opacity: 0;

        lifeSpan: 800
        angle: 360
        velocity: 20
        count:0;
        emissionRate: 50;
        fadeInDuration: 10;

        velocityDeviation: 10
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
