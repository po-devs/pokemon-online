import QtQuick 1.1

/* Should be called with those parameters;
  - curve: the function that gives x and y depending on percent: curve.x(percent) and curve.y(percent)
  - delay: the delay after which to fire the finished signal once the moving animation finished
  - duration: the duration of the moving
  */
AnimatedImage {
    id: image;

    property variant curve;
    paused: true;

    Curve {
        id: _curve;
        pos1: curve.pos1;
        pos2: curve.pos2;
        controlY: curve.controlY;
    }

    property real percent: 0;
    property int duration;

    property int delay;

    x: _curve.x(percent);
    y: _curve.y(percent);
    z: _curve.z(percent);
    scale: calculateScale(_curve.z(percent));

    SequentialAnimation {
        id: anim
        NumberAnimation { target: image; property: "percent"; to: 1.0; duration: image.duration}
        ScriptAction {script: image.paused=false;}
        PauseAnimation { duration: delay }
        ScriptAction { script: image.finished();}
    }

    onCurrentFrameChanged: {
        if (currentFrame === frameCount-1) {
            paused = true;
        }
    }

    function start() {
        anim.start();
    }

    signal finished();
}
