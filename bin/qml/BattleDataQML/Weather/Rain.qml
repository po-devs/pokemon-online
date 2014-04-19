import QtQuick 2.0
import QtQuick.Particles 2.0

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

    ParticleSystem {
        parent: main.parent;
        y: 0;
        x: -100;
        width: main.parent.width+100
        height: 1;
        id: particles

        ImageParticle {
            source: "../../images/raindrop.png"
        }

        Emitter {
            lifeSpan: 1000
            velocity: AngleDirection {
                angle: 75; angleVariation: 10;
                magnitude: 500; magnitudeVariation: 10
            }
            emitRate: 80;
            enabled: parent.count > 0
            maximumEmitted: parent.count;
        }

        property int count: 0

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
