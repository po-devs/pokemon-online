import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Rectangle {
    width: 101
    height: 8
    color: "#ffffff"
    radius: 4
    border.color: "#0d0d0d"

    Rectangle {
        id: rectangle1
        x: 1
        y: 1
        width: pokemon.life
        height: 7
        color: "#1f6920"
        radius: 2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.topMargin: 1
    }
}
