import QtQuick 2.0
import QtGraphicalEffects 1.0
import pokemononline.battlemanager.proxies 1.0
import "Utilities" 1.0
import "colors.js" as Colors
import "effects.js" as Effects
import "moves.js" as Moves
import "utilities.js" as Utils

Item {
    id: fieldPoke
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
            //console.log("param: " + i + ": " + params[i]);
        }

        //battle.scene.debug("Using attack " + attack + "\n");
        Moves.useAttack(fieldPoke, attack, target, params);
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
        id: hp
        parent: fieldPoke.parent
        anchors.horizontalCenter: fieldPoke.horizontalCenter
        y: fieldPoke.y - 10 - ((calculateScale(fieldPoke.z)-1)*fieldPoke.height*0.8);
    }

    Text {
        id: nick
        parent: hp
        anchors.horizontalCenter: hp.horizontalCenter
        anchors.bottom: hp.top;
        text: pokemon.nick
    }

    AnimatedImage {
        id: aimg /* Dont use image, will cause problems if you do for the shader */
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        transformOrigin: Item.Bottom;
        property int forme: img.spriteRef >> 16;
        property bool femaleTry: true;

        source: fieldPokemon.showing ? ("http://pokemon-online.eu/images/pokemon/x-y/animated/" + (back?"back/":"") + (pokemon.shiny?"shiny/":"")
                                        + ((femaleTry && pokemon.gender==2)?"female/":"") + padd(img.spriteRef&0xFFFF) + (forme ? "-"+forme:"") + ".gif") : ""

        scale: img.scale
        anchors.bottomMargin: img.anchors.bottomMargin
        anchors.horizontalCenterOffset: img.anchors.horizontalCenterOffset
        z: img.z
        opacity: img.opacity
        smooth: false
        onStatusChanged: if (status === Image.Ready) {
                             shader.source = aimg;
                         } else {
                             shader.source = img;
                             if (status!=Image.Loading &&pokemon.gender==2&&femaleTry) {
                                femaleTry=false;
                             }
                         }
        Connections {
            target: img
            onSpriteRefChanged: function() {
                femaleTry = true;
            }
        }

        visible: status === Image.Ready
    }

    function padd(i) {
        return ("00"+i).slice(-3);
    }

    Image {
        property int spriteRef: fieldPokemon.alternateSprite || fieldPokemon.pokemon.numRef;
        property real baseScale: 1;
        id: img /* Dont use image, will cause problems if you do for the shader */
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        transformOrigin: Item.Bottom;
        source: fieldPokemon.showing ? "image://pokeinfo/pokemon/"+ spriteRef + "&gender="+pokemon.gender+"&back="+back+"&shiny="+pokemon.shiny+"&cropped=true" : ""

        scale: baseScale * calculateScale(fieldPoke.z+z)
        smooth: false
        visible: !aimg.visible
    }

    /**
      As we now use a shader overlapping an image, if the image
      has a certain opacity we need to reduce it even more because
      of the two overlapping images (image & shader) with same opacity.

      This function gives the actualy opacity to use in function of the
      final opacity we want */
    function calc_opacity(opac) {
        return 1 - Math.sqrt(1-opac);
    }

    Tooltip {
        id: tooltip
        shown: mouseArea.containsMouse || mouseArea2.containsMouse

        onShownChanged: {
            if (shown) {
                text = getTooltipText();
            }
        }

        MouseArea {
            id: mouseArea2
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            preventStealing: true
            z: 10000
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: img
        hoverEnabled: true
        propagateComposedEvents: true
        preventStealing: true
    }

    Image {
        id :substitute;
        opacity: 0;
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        source: "image://pokeinfo/pokemon/substitute&back="+back+"&cropped=true"
    }

    /* Used to display fainted pokemon */
    ColorOverlay {
        id: shader
        source: img
        anchors.fill: source
        color: transformColor(Colors.statusColor(pokemon.status))
        scale: source.scale
        transformOrigin: source.transformOrigin
        opacity: (pokemon.status === PokeData.Fine || pokemon.status === PokeData.Koed) ? 0.0 : 1.0

        function transformColor(color) {
            // add transparency
            color[1] = 'B'
            return color;
        }
    }

    property Image pokeSprite: img

    Connections {
        target: fieldPokemon
        onStatUp: {
            var level = battle.scene.statboostlevel();
            if (level > 0) {
                Effects.statUp(fieldPoke, level);
            }
        }
        onStatDown: {
            var level = battle.scene.statboostlevel();
            if (level > 0) {
                Effects.statDown(fieldPoke, level);
            }
        }
    }

    states: [
        State {
            name: "substitute"
            when: fieldPokemon.substitute
            extend: "onTheField"
            PropertyChanges {
                target: img
                anchors.bottomMargin: back ? -40 : 40;
                anchors.horizontalCenterOffset: back? -40: 40;
                opacity: calc_opacity(0.7);
                z: fieldPoke.back ? 10 : -10;
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
                target: img
                opacity: 0;
            }
        },

        State {
            name: "onTheField"
            when: fieldPokemon.onTheField
            PropertyChanges {
                target: img
                opacity: 1;
                baseScale: 1;
            }
        },

        State {
            name: "offTheField"
            when: !fieldPokemon.onTheField
            PropertyChanges {
                target: img
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
                        if (fieldPoke.pokemon.numRef===0) {
                            anim.stop();
                        } else {
                            //battle.scene.debug("Beginning ko animation for " + fieldPoke.pokemon.numRef + "\n");
                            battle.scene.pause();
                            battle.scene.playCry(fieldPoke.pokemon.numRef);
                        }
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        target: img; property: "opacity";
                        from: 1; to: 0; duration: 800
                    }
                    NumberAnimation {
                        target: img; property: "anchors.bottomMargin";
                        from: 0; to: -96; duration: 800;
                    }
                }
                PropertyAction { target: img; property: "anchors.bottomMargin"; value: 0 }
                /* Restores image state */
                ScriptAction {
                    script: {
                        //battle.scene.debug("Ending ko animation for " + fieldPoke.pokemon.numRef + "\n");
                        battle.scene.unpause();
                    }
                }
            }
        },
        Transition {
            from: "koed,offTheField"
            to: "onTheField"
            SequentialAnimation {
                PropertyAction { targets: [img, shader]; properties: "opacity"; value: 0 }
                ScriptAction { script: {
                        //battle.scene.debug("Beginning sendout animation for " + fieldPoke.pokemon.numRef + "\n");
                        battle.scene.pause();
                        pokeball.trigger(); } }
                PauseAnimation { duration: 800 }
                PropertyAction { targets: [img, shader]; properties: "opacity"; value: 1}
                PropertyAction { target: img; property: "anchors.bottomMargin"; value: 70}
                NumberAnimation { target:img; from: 0.5;
                    to: 1.0; property: "baseScale"; duration: 350; easing.type: Easing.InQuad }
                ScriptAction {script: battle.scene.playCry(fieldPoke.pokemon.numRef);}
                PauseAnimation { duration: 150 }
                NumberAnimation { target:img; from: 70;
                    to: 0; property: "anchors.bottomMargin"; duration: 400; easing.type: Easing.OutBounce}
                /* Grace pausing time after a pokemon is sent out*/
                NumberAnimation {duration: 300}
                ScriptAction {script: {
                        //battle.scene.debug("Ending  sendout animation for " + fieldPoke.pokemon.numRef + "\n");
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
                        //battle.scene.debug("Beginning sendback animation for " + fieldPoke.pokemon.numRef + "\n");
                        battle.scene.pause();}}
                NumberAnimation { target: img; property: "baseScale";
                    duration:400; from: 1.0; to: 0.5 ; easing.type: Easing.InQuad }
                NumberAnimation { property: "opacity";  duration: 200
                    from: 1.0; to: 0; target: img}
                ScriptAction {script: {img.baseScale = 1;
                        //battle.scene.debug("Ending sendback animation for " + fieldPoke.pokemon.numRef + "\n");
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
                    NumberAnimation {target: img; property: "anchors.horizontalCenterOffset"}
                    NumberAnimation {target: img; property: "anchors.bottomMargin"}
                    NumberAnimation {target: img; property: "z"}
                    NumberAnimation {targets: [img, shader]; property: "opacity"}
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
                    NumberAnimation {target: img; property: "anchors.horizontalCenterOffset"}
                    NumberAnimation {target: img; property: "anchors.bottomMargin"}
                    NumberAnimation {target: img; property: "z"}
                    NumberAnimation {targets: [img,shader]; property: "opacity"}
                }
                ScriptAction { script: battle.scene.unpause();}
            }
        }
    ]

    function getTooltipText () {
        console.log("poke: " + pokemon);
        console.log("Field poke: " + fieldPokemon);

        var s = pokemon.nick + " &nbsp; lv. " + pokemon.level;

        if (pokemon.gender === 1) {
            s += " (M)"
        } else if (pokemon.gender === 2) {
            s += " (F)"
        }

        s += "<br/>" + Utils.typeImg(fieldPokemon.type1());
        if (fieldPokemon.type2() !== 18) s += Utils.typeImg(fieldPokemon.type2());

        s += "<table><tr><td><table>";
        var stats = [qsTr("Attack"), qsTr("Defense"), qsTr("Sp. Attack."), qsTr("Sp. Defense"), qsTr("Speed"), qsTr("Accuracy"), qsTr("Evasion")];
        var boost,stat,i;

        for (i = 0; i < 5; i++) {
            s += "<tr><td>" + stats[i] + "</td><td>&nbsp;";
            stat = fieldPokemon.stat(i+1);
            boost = fieldPokemon.statBoost(i+1);

            if (stat === 0) {
                s += fieldPokemon.minStat(i+1) + "-" + fieldPokemon.maxStat(i+1) + "</td><td>"
            } else {
                if (stat === -1) {
                    s += "???"
                } else {
                    s += stat
                }
                s += "</td><td>"
            }
            if (boost > 0) {
                s += "(+" + boost + ")"
            } else if (boost < 0) {
                s += "(" + boost + ")"
            }
            s += "</td></tr>"
        }

        for (i = 5; i < 7; i++) {
            boost = fieldPokemon.statBoost(i+1);
            if (boost !==0) {
                s += "<tr><td colspan='2'>" + stats[i] + "</td><td>&nbsp;";

                if (boost > 0) {
                    s += "+" + boost;
                } else if (boost < 0) {
                    s += +boost;
                }

                s += "</td></tr>"
            }
        }

        s += "</table></td>"

        console.log("num moves " + pokemon.numMoves());
        if (!battle.scene.isPlayer(spot) && pokemon.numMoves() > 0 && pokemon.move(0).num !== 0) {
            s += "<td><br/>"
            for (i = 0; i < pokemon.numMoves(); i++) {
                var moveinfo = pokemon.move(i)

                console.log("move info: " + moveinfo);
                if (moveinfo) {
                    var move = moveinfo.num;
                    console.log("move: " + move + ", moveInfo: " + moveInfo)
                    if (move > 0) {
                        var type = moveInfo.type(move);
                        console.log("type: " + type);
                        s += "<b><span style='color: " + theme.typeColor(type) + ";'>" +
                                "- " + moveInfo.name(move) + " (" + pokemon.move(i).PP + "/" + pokemon.move(i).totalPP + " PPs)</span></b><br/>";
                    }
                }
            }
            s += "</td>"
        }

        s += "</tr>"

        var zone = battle.data.field.zone(spot%2);
        var hazards = [];
        if (zone.stealthRocks) {
            hazards.push("Stealth Rocks");
        }
        if (zone.stickyWeb) {
            hazards.push("Sticky Web");
        }
        if (zone.spikesLevel > 0) {
            hazards.push("Spikes level " + zone.spikesLevel);
        }
        if (zone.tspikesLevel > 0) {
            hazards.push("Toxic Spikes level " + zone.tspikesLevel);
        }

        if (hazards.length > 0) {
            s += "<tr><td>" + hazards.join(", ") + "</td></tr>";
        }

        s += "</table>"

        return s;
    }
}
