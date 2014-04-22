import QtQuick 2.0
import pokemononline.battlemanager.proxies 1.0

Rectangle {
    id: main;
    width: 130
    height: 9
    color: "#ffffff"
    radius: 2
    z: 500;
    border.color: "#0d0d0d"

    property int oldValue: 0;
    property bool trigger: false;
    property bool running: false;

    function onLifeChanged() {
        if (running) {
            return;
        }
        if (fieldPoke.pokemon.numRef===0 || main.oldValue === fieldPoke.pokemon.lifePercent || !fieldPoke.fieldPokemon.onTheField) {
            rectangle1.width = oldValue = fieldPoke.pokemon.lifePercent;
            return;
        }
        trigger = !trigger;
    }

    Connections {
        target: fieldPoke.pokemon
        onLifePercentChanged: {
            onLifeChanged();
        }
        onNumChanged: {
            onLifeChanged();
        }
    }

    Connections {
        target: fieldPoke;
        onPokemonChanged: {
            onLifeChanged();
        }
    }

    Behavior on trigger {
        SequentialAnimation {
            id: anim;
            ScriptAction {
                script: {
                    //battle.scene.debug("Beginning life animation for " + fieldPoke.pokemon.numRef + "\n");
                    battle.scene.pause();
                    main.running = true;
                    //battle.scene.debug("Old value: " + main.oldValue);
                    numanim.duration = Math.floor(Math.abs(fieldPoke.pokemon.lifePercent-main.oldValue) * 10);
                }
            }
            NumberAnimation {
                target: rectangle1
                property: "width"
                to: fieldPoke.pokemon.lifePercent
                id: numanim
            }
            PauseAnimation { duration: 100; }
            ScriptAction {
                script: {
                    main.oldValue = fieldPoke.pokemon.lifePercent;
                    rectangle1.width = main.oldValue;
                    //battle.scene.debug("Ending life animation for " + fieldPoke.pokemon.numRef + "\n");
                    main.running = false;
                    battle.scene.unpause();
                }
            }
        }
    }

    Rectangle {
        z: 4;
        id: rectangle1
        x: 1
        y: 1
        width: 0
        height: 7
        color: width >= 50 ? "#1fc42a": (width >= 25 ? "#f8db17" : "#b80202")
        radius: 1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.topMargin: 1
    }

    Rectangle {
        x: parent.width-29;
        z: 4;
        y: 0;

        border.color: "#0d0d0d"
        radius: 1
        width: 29;
        height: 9;
        color: "#0a2a5a";
        opacity: 0.9;

        Text {
            anchors.right: parent.right
            anchors.rightMargin: 2
            opacity: 1;
            x:4
            y: -1
            font.pixelSize: 9;
            font.bold: true;
            color: "white";
            text: Math.floor(rectangle1.width) + "%"
        }
    }

    Component.onCompleted: {
        oldValue = fieldPoke.pokemon.lifePercent;
        rectangle1.width = fieldPoke.pokemon.lifePercent;
    }
}
