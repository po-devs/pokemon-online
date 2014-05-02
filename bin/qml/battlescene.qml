import QtQuick 1.1
import pokemononline.battlemanager.proxies 1.0
import "BattleDataQML" 1.0
import "BattleDataQML/Utilities/" 1.0
import "BattleDataQML/weather.js" as Weather

Item {
    id: scene;
    width: bg.width;
    height: bg.height
    /* Separate element so that it can have a negative Z and be separate from the rest */
    Image {
        id: bg;
        source: "images/grass.png"
        width: battle.scene.width
        height: battle.scene.height
        z: -500;
    }

    property bool loaded: false;
    property int playerFront: battle.scene.reversed ? 0: 1;
    property int playerBack: battle.scene.reversed ? 1: 0;

    property variant fieldPokemons: battle.scene.reversed ? [poke2, poke1] : [poke1, poke2];

    property real closeScale: bg.width === 500 ? 1 : 1.5
    property real farScale: 1
    property real closePos: poke1.z
    property real farPos: poke2.z;

    function calculateScale(z) {
        return closeScale + (z-closePos) / (farPos-closePos) * (farScale-closeScale);
    }

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

    Logger {
        id: logger;
        width: scene.width;
        anchors.bottom: scene.bottom;
        anchors.bottomMargin: -15;
        shown: true;
    }

    Rectangle {
        z: 500
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5

        width: 130
        height: 25

        border.width: 2
        radius: 5
        border.color: "#6890f0"
        color: "transparent"

        Rectangle {
            z: -10
            anchors.fill: parent
            anchors.rightMargin: (300-battle.data.team(playerBack).time) * parent.width/300
            color: Qt.lighter(parent.border.color)
            radius: parent.radius
            opacity: 0.7
        }

        Text {
            anchors.centerIn: parent
            text: battle.data.team(playerBack).name
            color: Qt.darker(parent.border.color, 3)
        }

        MouseArea {
            id: mouseAreaBack
            anchors.fill: parent
            hoverEnabled: true
        }

        Tooltip {
            shown: mouseAreaBack.containsMouse
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            x: 2
            width: parent.width - 4
            text: Math.floor(battle.data.team(playerBack).time / 60) + ":" + (battle.data.team(playerBack).time % 60 < 10 ? "0" : "") + battle.data.team(playerBack).time % 60;
        }
    }

    Rectangle {
        z: -100
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 5

        width: 130
        height: 25

        opacity: 0.7
        border.width: 2
        radius: 5
        border.color: "#b04924"
        color: "transparent"

        Rectangle {
            z: -10
            anchors.fill: parent
            anchors.rightMargin: (300-battle.data.team(playerFront).time) * parent.width/300
            color: "#ecb6a3"
            radius: parent.radius
            opacity: 0.7
        }

        Text {
            anchors.centerIn: parent
            text: battle.data.team(playerFront).name
            color: Qt.darker(parent.border.color, 5)
        }

        MouseArea {
            id: mouseAreaFront
            anchors.fill: parent
            hoverEnabled: true
        }

        Tooltip {
            shown: mouseAreaFront.containsMouse
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            x: 2
            width: parent.width - 4
            text: Math.floor(battle.data.team(playerFront).time / 60) + ":" + (battle.data.team(playerFront).time % 60 < 10 ? "0" : "") + battle.data.team(playerFront).time % 60;
        }
    }

    Rectangle {
        z: 500 // this appears to be in front, and the other in the back. just a little perspective thing
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 36

        width: 96
        height: 96

        color: "#8888FF"
        opacity: 0.7

        border.width: 1
        border.color: "#0000FF"
        radius: 5

        Image {
            source: "file:///" + theme.trainerSpritePath(battle.data.avatar(playerBack))
            anchors.centerIn: parent
        }
    }

    Rectangle {
        z: -100
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 35

        width: 96
        height: 96

        color: "#FF8888"
        opacity: 0.7

        border.width: 1
        border.color: "#FF0000"
        radius: 5

        Image {
            source: "file:///" + theme.trainerSpritePath(battle.data.avatar(playerFront))
            anchors.centerIn: parent
        }
    }

    Team {
        id: team1
        anchors.right: parent.right
        anchors.rightMargin: bg.width === 500 ? 20 : 50
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 96
        height: 64
        team: battle.data.team(playerBack)
    }

    Team {
        id: team2
        team: battle.data.team(playerFront)
        width: 96
        height: 64
        anchors.left: parent.left
        anchors.leftMargin: bg.width === 500 ? 20 : 50
        anchors.top: parent.top
        anchors.topMargin: 20
    }

    FieldPokemon {
        id: poke1
        back: true
        spot: playerBack;
        fieldPokemon: battle.data.field.poke(playerBack)
        pokemon: team1.team.poke(0)
        anchors.left: parent.left
        anchors.leftMargin: bg.width === 500 ? 55 : 90
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 55
    }

    FieldPokemon {
        id: poke2
        back: false
        spot: playerFront;
        fieldPokemon: battle.data.field.poke(playerFront)
        pokemon: team2.team.poke(0)
        anchors.right: parent.right
        anchors.rightMargin: bg.width === 500 ? 65 : 115
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

    Connections {
        target: battle.data.field
        onWeatherChanged: {
            Weather.trigger(weather, battle.data.field.weather);
        }
    }

    Connections {
        target: battle.scene
        onAttackUsed: {
            fieldPokemons[spot].useAttack(attack, fieldPokemons[1-spot], params);
        }
        onHit: {
            fieldPokemons[spot].useAttack(attack, fieldPokemons[1-spot], params);
        }
        onBattleLog: {
            if (battle.scene.option("logger")) logger.log(logMessage);
        }
        onWeatherContinue: {
            if (battle.scene.option("weather") == "always") Weather.trigger(weather, battle.data.field.weather);
        }
    }
}
