import QtQuick 2.0

FrameAnimation {
    id: pokeball;

    source: "../images/pokeballs.png"
    width: 41
    height: 40
    frameCount: 8
    delay: 50
    col: 6
    scale: calculateScale(fieldPoke.z)

    function trigger() {
        //console.log("y: " + y + ", x: " + x);
        triggerProp = !triggerProp;
    }

    property bool triggerProp: false

    parent: fieldPoke.parent;

    property int destX: fieldPoke.x + (fieldPoke.width/2-20);
    property int destY: fieldPoke.y - 20;

    property int deltaX: fieldPoke.back ? - 170 : 180
    property int deltaY: fieldPoke.back ? 100 : 20

    property int sourceX: destX + deltaX;
    property int sourceY: destY + deltaY;

    property int amplY: 60;

    x: sourceX - deltaX * percent;
    y: sourceY + (0.5-percent)*(0.5-percent)*4*amplY - amplY - percent*deltaY;

    property real percent: 0;

    signal animationCompleted();

    Behavior on triggerProp {
        SequentialAnimation {
            ScriptAction {
                script: {pokeball.reset();}
            }
            PropertyAction { target: pokeball; property: "paused"; value: false }
            PropertyAction { target: pokeball; property: "opacity"; value: 1 }
            NumberAnimation {
                target: pokeball; property: "percent"
                from: 0.2; to: 1.0; duration: 700;
            }

            NumberAnimation {
                target: pokeball; property: "opacity"
                from: 1.0; to: 0; duration: 150;
            }
            PropertyAction { target: pokeball; property: "paused"; value: true }
            ScriptAction {
                script: { animationCompleted();}
            }
        }
    }
}
