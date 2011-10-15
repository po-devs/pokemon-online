import QtQuick 1.0
import "../" 1.0
import "../utilities.js" as Utilities
import "../spawner.js" as Spawner

Item {
    id: main;

    /* Available variables:
      - attacker (FieldPokemon)
      - target (FieldPokemon)
      - attack (int)
      */
    function start() {
        battle.scene.pause();

        var curve1 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":defender.x, "y":defender.y-10}, 120);
        var curve2 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":defender.x+30, "y":defender.y-30}, 120);
        var curve3 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":defender.x+70, "y":defender.y+5}, 120);

        var parent = main.parent;
        leech1 = Spawner.spawn(parent, "moving-animated", {
                                   "source":"../../images/leech-seed.gif",
                                   duration: 350,
                                   delay: 50,
                                   curve: curve1
                               },
                               function(obj){
                                   obj.destroy();
                                   battle.scene.unpause();
                                   finished();
                               }
                               );
    }

    signal finished();
}
