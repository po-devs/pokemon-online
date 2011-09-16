import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0

Item {
    property bool back: false
    property FieldPokeData fieldPokemon
    property PokeData pokemon

    function isKoed() {
        return pokemon.status === 31;
    }

    property bool wasKoed: false
    property bool koed: isKoed()
    property bool onTheField: fieldPokemon.onTheField

    property bool triggerKo: false
    property bool triggerSendBack: false
    property bool triggerSendOut: false

    onKoedChanged: {
        if (wasKoed == koed) {
            return;
        }
        wasKoed = koed;

        if (!fieldPokemon.onTheField) {
            return;
        }

        if (koed) {
            triggerKo = !triggerKo;
        } else {
            triggerSendOut = !triggerSendOut;
        }
    }

    onOnTheFieldChanged: {
        if (pokemon.isKoed()) {
            return;
        }
        if (onTheField) {
            triggerSendOut = !triggerSendOut;
        } else {
            triggerSendBack = !triggerSendBack;
        }
    }

    width: 96
    height: 96

    Image {
        id: image
        source: "image://pokeinfo/pokemon/"+pokemon.numRef+"&back="+back+"&shiny="+pokemon.shiny
    }

    Behavior on triggerKo {
        SequentialAnimation {
            ScriptAction {
                script: {battle.scene.pause();}
            }
            ParallelAnimation {
                NumberAnimation {
                    target: image; property: "opacity";
                    from: 1; to: 0; duration: 800
                }
                NumberAnimation {
                    target: image; property: "y";
                    from: image.y; to: image.y+96; duration: 800;
                }
            }
            /* Restores image state */
            ScriptAction {
                script: {image.y -= 96; battle.scene.unpause();}
            }
        }
    }

    /* When a pokemon is sent out */
    Behavior on triggerSendOut {
        animation: SequentialAnimation {
            ScriptAction { script: {battle.scene.pause(); image.opacity = 1;} }
            NumberAnimation { target:image; from: 0.5;
                to: 1.0; property: "scale"; duration: 400 }
            /* Grace pausing time after a pokemon is sent out*/
            NumberAnimation {duration: 300}
            ScriptAction {script: battle.scene.unpause();}
        }
    }

    /* When a pokemon is sent back */
    Behavior on triggerSendBack {
        animation: SequentialAnimation {
            ScriptAction {script: battle.scene.pause();}
            NumberAnimation { target: image; property: "scale";
                duration: 600; from: 1.0; to: 0.5 }
            NumberAnimation { property: "opacity";  duration: 200
                from: 1.0; to: 0; target: image}
            ScriptAction {script: battle.scene.unpause();}
        }
    }
}
