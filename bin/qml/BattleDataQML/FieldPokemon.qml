import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    property bool back: false
    property FieldPokeData fieldPokemon
    property PokeData pokemon

    width: 96
    height: 96

    Image {
        source: "image://pokeinfo/pokemon/"+pokemon.numRef+"&back="+back
    }
}
