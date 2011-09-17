import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0
import Qt.labs.shaders 1.0
Item {
    id: main
    width: grid.cellWidth
    height: grid.cellHeight

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
    }

    /* Used to display fainted pokemon */
    ColorShader {
        id: shader
        image: img
        blendColor: "black"
        alpha: pokemon.status === 31 ? 0.5 : 0
    }

}
