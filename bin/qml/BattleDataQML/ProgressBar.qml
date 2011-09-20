import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Rectangle {
    id: main;
    width: 101
    height: 8
    color: "#ffffff"
    radius: 4
    border.color: "#0d0d0d"

    property int oldValue: 0;

    Rectangle {
        id: rectangle1
        x: 1
        y: 1
        width: woof.pokemon.lifePercent
        height: 7
        color: width >= 50 ? "#1fc42a": (width >= 25 ? "#f8db17" : "#b80202")
        radius: 2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.topMargin: 1

        Behavior on width {
            SequentialAnimation {
                id: anim;
                ScriptAction {
                    script: {
                        if (woof.pokemon.numRef===0) {
                            anim.complete();
                            return;
                        }

                        battle.scene.pause();
                        //battle.scene.debug("Beginning animation for " + woof.pokemon.numRef + "\n");
                        numanim.duration = Math.floor(Math.abs(woof.pokemon.lifePercent-main.oldValue) * 10);
                    }
                }
                NumberAnimation {
                    id: numanim
                }
                ScriptAction {
                    script: {
                        //battle.scene.debug("Ending animation for " + woof.pokemon.numRef + "\n");
                        main.oldValue = woof.pokemon.lifePercent; battle.scene.unpause();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        oldValue = woof.pokemon.lifePercent;
    }
}
