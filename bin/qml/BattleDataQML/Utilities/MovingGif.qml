import QtQuick 1.0

/* Should be called with those parameters;
  - curve: the function that gives x and y depending on percent: curve.x(percent) and curve.y(percent)
  - delay: the delay after which to fire the finished signal once the moving animation finished
  - duration: the duration of the moving
  */
AnimatedImage {
    id: image;
    property real percent: 0;
    property int duration;
    property variant curve;
    property int delay;

    x: curve.x(percent);
    y: curve.y(percent);

    SequentialAnimation {
        id: anim
        NumberAnimation { target: image; property: "percent"; to: 1.0; duration: image.duration}
        PauseAnimation { duration: delay }
        ScriptAction { script: image.finished();}
    }

    function start() {
        anim.start();
    }

    signal finished();
}
