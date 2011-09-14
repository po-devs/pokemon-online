import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0
import "BattleDataQML" 1.0

Rectangle {
    width: 500
    height: 400
    property bool loaded: false;
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#cfa50d"
        }

        GradientStop {
            position: 0.460
            color: "#9e7373"
        }

        GradientStop {
            position: 0.700
            color: "#7794a6"
        }

        GradientStop {
            position: 1
            color: "#40598d"
        }
    }

    Rectangle {
        id: team1
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        color:"transparent"
        width: 96
        height: 64
        Team {
            player: 0
            anchors.fill: parent
        }
    }

    Rectangle {
        id: team2
        color: "transparent"
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        width: 96
        height: 64
        Team {
            player: 1
            anchors.fill: parent
        }
    }
}
