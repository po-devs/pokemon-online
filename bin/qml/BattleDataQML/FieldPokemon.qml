import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    property bool back: false

    Image {
        source: "image://pokeinfo/pokemon/1&back="+back
    }
}
