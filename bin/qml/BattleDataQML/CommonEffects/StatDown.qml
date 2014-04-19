import QtQuick 2.0
import QtQuick.Particles 2.0
import "../" 1.0

Item {
    id: main;
    property FieldPokemon pokemon
    property int level

    ParticleSystem {
        id: particles;

        parent: pokemon.pokeSprite.parent
        x: pokemon.pokeSprite.x + pokemon.pokeSprite.width/2 - width/2
        y: pokemon.pokeSprite.y+5;
        width: pokemon.pokeSprite.width/2
        height: 15;

        ImageParticle {
            source: "../../images/bluecircle.png"
        }

        Emitter {
            lifeSpan: 1300
            lifeSpanVariation: 100;

            velocity: AngleDirection {
                angle: 90; angleVariation: 60;
                magnitude: 60; magnitudeVariation: 10;
            }

            emitRate: 40*level
            maximumEmitted: parent.count
            enabled: parent.count > 0
        }

        property int count: 0

        z: pokemon.pokeSprite.z + 1;

        scale: calculateScale(z+pokemon.z)
        transformOrigin: Item.Bottom
    }

    SequentialAnimation {
        id: anim
        ScriptAction {
            script: {battle.scene.pause(); particles.count=40*level; }
        }
        PauseAnimation { duration: 500 }
        ScriptAction {
            script: {particles.count = 0; }
        }
        PauseAnimation {duration: 800;}
        ScriptAction {
            script: {battle.scene.unpause();}
        }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: main.finished();}
    }

    function start() {
        anim.start();
    }

    signal finished();
}
