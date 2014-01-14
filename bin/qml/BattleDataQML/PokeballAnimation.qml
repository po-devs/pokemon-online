import QtQuick 1.1

FrameAnimation {
    id: pokeball;

    source: "../images/pokeballs.png"
    width: 41
    height: 40
    frameCount: 8
    delay: 50
    col: 6
    scale: calculateScale(woof.z)

    function trigger() {
        //console.log("y: " + y + ", x: " + x);
        triggerProp = !triggerProp;
    }

    property bool triggerProp: false

    parent: woof.parent;

    property int destX: woof.x + (woof.width/2-20);
    property int destY: woof.y - 20;

    property int deltaX: woof.back ? - 170 : 180
    property int deltaY: woof.back ? 100 : 20

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
