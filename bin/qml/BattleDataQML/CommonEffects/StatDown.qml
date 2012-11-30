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
        y: pokemon.pokeSprite.y+5;
        width: pokemon.pokeSprite.width/2
        height: 15;
        source: "../../images/bluecircle.png"

        lifeSpan: 1300
        lifeSpanDeviation: 100;
        angle: 90
        angleDeviation: 60
        velocity: 60
        count: 0;
        emissionRate: 40*level;

        velocityDeviation: 10
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
