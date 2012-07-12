import QtQuick 1.1
import pokemononline.battlemanager.proxies 1.0
import Qt.labs.shaders 1.0
import "colors.js" as Colors
import "Utilities" 1.0

Item {
    id: main
    width: grid.cellWidth
    height: grid.cellHeight
    z: 100;

    Image {
        id: img
        x: main.x
        y: main.y

        /* In order to use animations, we can't use the main level component as
          it technically is the same item, so we need to animate the image.

          So we set the image's position relative to the parent instead, so
          we can animate its coordinates properly */
        parent: grid

        Behavior on x { enabled: grid.loaded; NumberAnimation { duration: 400; easing.type: Easing.InOutCubic}}
        Behavior on y { enabled: grid.loaded; NumberAnimation { duration: 400; easing.type: Easing.InOutCubic}}

        source: pokemon.numRef === 0 ? "../images/pokeballicon.png" : ("image://pokeinfo/icon/"+ pokemon.numRef)
        width: 32
        height: 32

        onSourceChanged: shader.grab()
        opacity: pokemon.status === PokeData.Koed ? 0.3 : (pokemon.status === PokeData.Fine? 1 : 0.7)
    }

    /* Used to display fainted pokemon */
    ColorShader {
        id: shader
        image: img
        blendColor: Colors.statusColor(pokemon.status)
        alpha: pokemon.status === 0 ? 0.0 : 0.5
        opac: 1
    }

    Tooltip {
        shown: mouseArea.containsMouse
        text: pokemon.numRef === 0 ? "" : pokemon.nick + " - " + pokemon.lifePercent + "%"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
