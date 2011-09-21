import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0
import "BattleDataQML" 1.0

Item {
    id: scene;

    width: 500
    height: 400

    /* Separate element so that it can have a negative Z and be separate from the rest */
    Image {
        source: "images/grass.png"
        anchors.fill: parent;
        z: -100;
    }

    property bool loaded: false;

    /* Rectangle used by the weather */
    Rectangle{
        id: weatherOverlay;
        anchors.fill: parent;
        opacity: 0;

        Behavior on opacity {
            NumberAnimation {
                duration: 250;
            }
        }
    }

    Team {
        id: team1
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 96
        height: 64
        team: battle.data.team(0)
    }

    Team {
        id: team2
        team: battle.data.team(1)
        width: 96
        height: 64
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
    }

    FieldPokemon {
        id: poke1
        back: true
        fieldPokemon: battle.data.field.poke(0)
        pokemon: team1.team.poke(0)
        anchors.left: parent.left
        anchors.leftMargin: 55
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 55
    }

    FieldPokemon {
        id: poke2
        back: false
        fieldPokemon: battle.data.field.poke(1)
        pokemon: team2.team.poke(0)
        anchors.right: parent.right
        anchors.rightMargin: 65
        anchors.top: parent.top
        anchors.topMargin: 55
    }

    Connections {
        target: team1
        onFieldPokemonChanged: {
            poke1.pokemon = team1.team.poke(pokemon)
        }
    }

    Connections {
        target: team2
        onFieldPokemonChanged: {
            poke2.pokemon = team2.team.poke(pokemon)
        }
    }

    Loader {
        anchors.fill: parent;
        id: weather
    }

    Component.onCompleted: {
        //battle.scene.unpause();

        //uncomment to start hail
        /* weather.source = "BattleDataQML/Weather/Hail.qml"
        weather.item.start();*/
    }
}
