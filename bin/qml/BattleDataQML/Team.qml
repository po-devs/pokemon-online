import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    property int player: 0
    property TeamData team: battle.data.team(player)

    Grid {
        anchors.fill: parent
        columns: 3
        spacing: 12
        Repeater {
            model: [team.poke(0), team.poke(1), team.poke(2), team.poke(3), team.poke(4), team.poke(5)]
            Pokemon {}
        }
    }
}
