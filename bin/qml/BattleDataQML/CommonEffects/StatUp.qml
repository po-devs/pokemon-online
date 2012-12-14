import QtQuick 1.1
import Qt.labs.particles 1.0
import "../" 1.0

Item {
    id: main;
    property FieldPokemon pokemon
    property int level

    Particles {
        id: particles;

        parent: pokemon.pokeSprite.parent
        x: pokemon.pokeSprite.x + pokemon.pokeSprite.width/2 - width/2
        y: pokemon.pokeSprite.y+pokemon.pokeSprite.height-pokemon.pokeSprite.height*pokemon.pokeSprite.scale/2;
        width: 2
        height: 2;
        source: "../../images/orangecircle.png"

        lifeSpan: 1400
        lifeSpanDeviation: 100;
        angle: 270
        angleDeviation: 50
        velocity: 100
        count: 0;
        emissionRate: 100*level;

        velocityDeviation: 20
        z: pokemon.pokeSprite.z + 1;

        ParticleMotionGravity {
            yattractor: 1000
            acceleration: 100
        }

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
            script: {particles.count = 0; particles.emissionRate=0; }
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
