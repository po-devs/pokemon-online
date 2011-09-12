import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    width: 32
    height: 32

    Image {
        id: pokeicon
        source: "image://pokeinfo/icon/"+ modelData.num
        width: 32
        height: 32
    }
}
