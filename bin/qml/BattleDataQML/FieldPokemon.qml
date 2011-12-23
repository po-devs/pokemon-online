import QtQuick 1.0
import pokemononline.battlemanager.proxies 1.0
import "colors.js" as Colors
import "effects.js" as Effects
import "moves.js" as Moves

Item {
    id: woof
    property bool back: false
    property FieldPokeData fieldPokemon
    property PokeData pokemon
    property int spot
    z: back ? 80 : 0;

    function isKoed() {
        return pokemon.status === 31 || pokemon.numRef === 0;
    }

    function useAttack(attack, target, params) {
        for (var i in params) {
            console.log("param: " + i + ": " + params[i]);
        }

        //battle.scene.debug("Using attack " + attack + "\n");
        Moves.useAttack(woof, attack, target, params);
    }

    function behind(zdelta) {
        return z + (back ? zdelta : -zdelta);
    }

    function infront(zdelta) {
        return z + (back ? -zdelta : zdelta);
    }


    PokeballAnimation {
        id: pokeball;
        paused: true;
        opacity: 1;
    }

    width: 96
    height: 96

    ProgressBar {
        parent: woof.parent
        x: woof.x
        y: woof.y - 30;
    }

    Image {
        property int spriteRef: fieldPokemon.alternateSprite || fieldPokemon.pokemon.numRef;
        id: image
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        transformOrigin: Item.Bottom
//        source: fieldPokemon.isShowing ? "image://pokeinfo/pokemon/"+
//                                         (fieldPokemon.alternateSpriteRef == 0 ? pokemon.numRef :fieldPokemon.alternateSpriteRef) +
//                                         "&gender="+pokemon.gender+"&back="+back+"&shiny="+pokemon.shiny : ""
        source: fieldPokemon.showing ? "image://pokeinfo/pokemon/"+ spriteRef + "&gender="+pokemon.gender+"&back="+back+"&shiny="+pokemon.shiny : ""
//        source: "image://pokeinfo/pokemon/"+ pokemon.numRef + "&gender="+pokemon.gender+"&back="+back+"&shiny="+pokemon.shiny

        onSourceChanged: shader.grab();
    }

    Image {
        id :substitute;
        opacity: 0;
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        source: "image://pokeinfo/pokemon/substitute&back="+back
    }

    /* Used to display fainted pokemon */
    ColorShader {
        id: shader
        image: image
        blendColor: Colors.statusColor(pokemon.status)
        alpha: (pokemon.status === PokeData.Fine || pokemon.status == PokeData.Koed) ? 0.0 : 0.3
    }

    property Image pokeSprite: image

    Connections {
        target: fieldPokemon
        onStatUp: {
            var level = battle.scene.statboostlevel();
            if (level > 0) {
                Effects.statUp(woof, level);
            }
        }
        onStatDown: {
            var level = battle.scene.statboostlevel();
            if (level > 0) {
                Effects.statDown(woof, level);
            }
        }
    }

    states: [
        State {
            name: "substitute"
            when: fieldPokemon.substitute
            extend: "onTheField"
            PropertyChanges {
                target: image
                anchors.bottomMargin: back ? -40 : 40;
                anchors.horizontalCenterOffset: back? -40: 40;
                opacity: 0.7;
                z: woof.back ? 10 : -10;
            }
            PropertyChanges {
                target: substitute
                opacity: 1;
            }
        },

        State {
            name: "koed"
            when: isKoed()
            PropertyChanges {
                target: image
                opacity: 0;
            }
        },

        State {
            name: "onTheField"
            when: fieldPokemon.onTheField
            PropertyChanges {
                target: image
                opacity: 1;
                scale: 1;
            }
        },

        State {
            name: "offTheField"
            when: !fieldPokemon.onTheField
            PropertyChanges {
                target: image
                opacity: 0;
            }
        }
    ]

    transitions: [
        Transition {
            from: "onTheField"
            to: "koed"
            SequentialAnimation {
                id: anim;
                ScriptAction {script: {
                        if (woof.pokemon.numRef===0) {
                            anim.stop();
                            return;
                        }

                        //battle.scene.debug("Beginning ko animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.pause();
                        battle.scene.playCry(woof.pokemon.numRef);
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        target: image; property: "opacity";
                        from: 1; to: 0; duration: 800
                    }
                    NumberAnimation {
                        target: image; property: "anchors.bottomMargin";
                        from: 0; to: -96; duration: 800;
                    }
                }
                PropertyAction { target: image; property: "anchors.bottomMargin"; value: 0 }
                /* Restores image state */
                ScriptAction {
                    script: {
                        //battle.scene.debug("Ending ko animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.unpause();
                    }
                }
            }
        },
        Transition {
            from: "koed,offTheField"
            to: "onTheField"
            SequentialAnimation {
                PropertyAction { targets: [image, shader]; properties: "opacity"; value: 0 }
                ScriptAction { script: {
                        //battle.scene.debug("Beginning sendout animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.pause();
                        pokeball.trigger(); } }
                PauseAnimation { duration: 800 }
                PropertyAction { targets: [image, shader]; properties: "opacity"; value: 1}
                PropertyAction { target: image; property: "anchors.bottomMargin"; value: 70}
                NumberAnimation { target:image; from: 0.5;
                    to: 1.0; property: "scale"; duration: 350; easing.type: Easing.InQuad }
                ScriptAction {script: battle.scene.playCry(woof.pokemon.numRef);}
                PauseAnimation { duration: 150 }
                NumberAnimation { target:image; from: 70;
                    to: 0; property: "anchors.bottomMargin"; duration: 400; easing.type: Easing.OutBounce}
                /* Grace pausing time after a pokemon is sent out*/
                NumberAnimation {duration: 300}
                ScriptAction {script: {
                        //battle.scene.debug("Ending  sendout animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.unpause();
                    }
                }
            }
        },
        Transition {
            from: "onTheField"
            to: "offTheField"
            SequentialAnimation {
                ScriptAction {script: {
                        //battle.scene.debug("Beginning sendback animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.pause();}}
                NumberAnimation { target: image; property: "scale";
                    duration:400; from: 1.0; to: 0.5 ; easing.type: Easing.InQuad }
                NumberAnimation { property: "opacity";  duration: 200
                    from: 1.0; to: 0; target: image}
                ScriptAction {script: {image.scale = 1;
                        //battle.scene.debug("Ending sendback animation for " + woof.pokemon.numRef + "\n");
                        battle.scene.unpause();}}
            }
        },
        Transition {
            from: "onTheField"
            to: "substitute"
            SequentialAnimation {
                ScriptAction { script: battle.scene.pause();}
                ParallelAnimation {
                    SequentialAnimation {
                        NumberAnimation { target: substitute; property: "anchors.bottomMargin"; to: 50; duration: 200
                        easing.type: Easing.OutQuad;}
                        NumberAnimation { target: substitute; property: "anchors.bottomMargin"; to: 0; duration: 200
                        easing.type: Easing.InCubic;}
                    }
                    NumberAnimation { target: substitute; property: "opacity";}
                    NumberAnimation {target: image; property: "anchors.horizontalCenterOffset"}
                    NumberAnimation {target: image; property: "anchors.bottomMargin"}
                    NumberAnimation {target: image; property: "opacity"}
                }
                ScriptAction { script: battle.scene.unpause();}
            }
        },
        Transition {
            from: "substitute"
            to: "onTheField"
            SequentialAnimation {
                ScriptAction { script: battle.scene.pause();}
                NumberAnimation { target: substitute; property: "opacity"}
                ParallelAnimation {
                    NumberAnimation {target: image; property: "anchors.horizontalCenterOffset"}
                    NumberAnimation {target: image; property: "anchors.bottomMargin"}
                    NumberAnimation {target: image; property: "opacity"}
                }
                ScriptAction { script: battle.scene.unpause();}
            }
        }
    ]
}
