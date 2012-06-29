import QtQuick 1.1
import "../" 1.0

Move {
    id: main;

    /*
     * The main animation of Earthquake.
     */
    SequentialAnimation  {
        id: animation;
        // Shake first 10 times
        SequentialAnimation  {
            loops: 10;
            NumberAnimation { target: scene; property: "x"; to: +50; duration: 50 }
            NumberAnimation { target: scene; property: "x"; to: -50; duration: 50 }
        }
        // Finally center it  
        NumberAnimation { target: scene; property: "x"; to: 0; duration: 100 }
        // Delete the Move object
        ScriptAction { script: finished(); }
    }

    function start() {
        animation.running = true;
    }
}
