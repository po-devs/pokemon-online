import QtQuick 2.0
import QtQuick.Particles 2.0

Move {
    id: main
    z: 300;

    Rectangle {
        id: overlay;
        parent: main.parent.parent
        anchors.fill: main.parent.parent
        opacity: 0.0;
        color: "#5a69b5";
    }

    property int xt: defender.x+defender.width/2-wave.width/2;
    property int yt: defender.y+defender.height-wave.height;

    property int x0: attacker.x+attacker.width/2-wave.width/2;
    property int y0: attacker.y+attacker.height-wave.height;

    property int z0: attacker.behind(1);
    property int zt: defender.behind(1);

    Image {
        property real progress: -0.1;
        id: wave;

        source: woof.back ? "../../images/surf-back.png" : "../../images/surf-front.png";
        parent: main.parent.parent

        x: (x0 + (xt-x0)*progress);
        y: (y0 + (yt-y0)*progress);
        z: (z0 + (zt-z0)*progress);
        scale: calculateScale(z);

        width: implicitWidth*(2-progress);
        height: implicitHeight*(0.7+progress)

        ParticleSystem {
            anchors.fill: parent

            ImageParticle {
                source: "../../images/flame0.png"
            }
            Emitter {
                lifeSpan: 750
                maximumEmitted: params.flames ? 6 : 0
                velocity: AngleDirection {
                   angle: woof.back ? -45 : (45+90)
                   angleVariation: 30;
                   magnitude: 4;
                   magnitudeVariation: 2;
                }
            }
        }
    }

    SequentialAnimation {
        id: animation;
        running: false;
        ParallelAnimation {
            NumberAnimation {
                target: overlay; duration: 250;
                property: "opacity"; to: 0.6;
            }
            NumberAnimation { target: wave; property: "progress"; duration: 1700; easing.type: Easing.InOutQuad; to: 1.6 }
        }
        NumberAnimation {
            targets: overlay; duration: 250;
            property: "opacity"; to: 0.0;
        }
        ScriptAction {
            script: finished();
        }
    }
}
