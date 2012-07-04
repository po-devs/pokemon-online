import QtQuick 1.1

/* Loads an animation from a PNG.

  The PNG is supposed to be one with each frame next to each other, aligned vertically or horizontally.
*/
Item {
    id: root

    clip: true
    property string source
    property int row: 0
    property int col: 0
    property int currentFrame: 0
    property int frameCount: 1
    property int delay: 100
    property bool paused: true

    function start() {
        paused = false
    }

    function pause() {
        paused = true
    }

    function reset() {
        currentFrame = 0
    }

    Timer {
        id: timer
        interval: delay
        running: !paused
        repeat: true
        onTriggered: currentFrame = (currentFrame + 1) % frameCount
    }

    Image {
        source: root.source
        x: -col * root.width
        y: -(row+currentFrame) * root.height
    }
}
