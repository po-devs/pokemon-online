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
        y: pokemon.pokeSprite.y+pokemon.pokeSprite.height-pokemon.pokeSprite.height*pokemon.pokeSprite.scale/2;
        width: 2
        height: 2;

        ImageParticle {
            source: "../../images/orangecircle.png"
        }

        Emitter {
            lifeSpan: 1400
            lifeSpanVariation: 100;

            /* Make it fall */
            acceleration: AngleDirection {
                angle: 90; magnitude: 100;
            }

            velocity: AngleDirection {
                angle: 270; angleVariation: 50;
                magnitude: 100; magnitudeVariation: 20;
            }

            emitRate: 100*level
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
            script: {battle.scene.pause(); particles.count=50*level; }
        }
        PauseAnimation { duration: 500 }
        ScriptAction {
            script: {particles.count = 0;  }
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
