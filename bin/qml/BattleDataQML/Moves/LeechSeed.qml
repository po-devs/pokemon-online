import QtQuick 1.0
import "../" 1.0
import "../utilities.js" as Utilities
import "../spawner.js" as Spawner

Move {
    id: main;

    /* Available variables:
      - attacker (FieldPokemon)
      - target (FieldPokemon)
      - attack (int)
      */
    function start() {
        battle.scene.pause();

        var curve1 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":target.x, "y":target.y-10}, 120);
        var curve2 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":target.x+30, "y":target.y-30}, 120);
        var curve3 = Utilities.makeCurve({"x":attacker.x, "y":attacker.y}, {"x":target.x+70, "y":target.y+5}, 120);

        var parent = main.parent;
        var leech1 = Spawner.spawn(parent, "moving-animated", {
                                   "source": "../../images/leech-seed.gif",
                                   "duration": 350,
                                   "delay": 50,
                                   "curve": curve1
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
