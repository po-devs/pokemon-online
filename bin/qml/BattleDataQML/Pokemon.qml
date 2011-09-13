import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    width: 32
    height: 32

    property PokeData poke: modelData
    Image {
        id: img
        source: "image://pokeinfo/icon/"+ poke.numRef
        width: 32
        height: 32
    }
}
