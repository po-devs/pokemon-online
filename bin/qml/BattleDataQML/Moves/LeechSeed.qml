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

        var curve1 = {"pos1":{"x":attacker.x+40, "y":attacker.y+10}, "pos2":{"x":target.x, "y":target.y+40}, "controlY":80};
        var curve2 = {"pos1":{"x":attacker.x+30, "y":attacker.y+5}, "pos2":{"x":target.x+30, "y":target.y+20}, "controlY":80};
        var curve3 = {"pos1":{"x":attacker.x+40, "y":attacker.y+15}, "pos2":{"x":target.x+70, "y":target.y+60}, "controlY":70};

//        var parent = main.parent;
//        var leech1 = Spawner.spawn(parent.parent, "moving-animated", {
//                                   "source": "../../images/leech-seed.gif",
//                                   "duration": 750,
//                                   "delay": 850,
//                                   "pos1": curve1.pos1,
//                                   "pos2": curve1.pos2,
//                                   "controlY": curve1.controlY
//                               },
//                               function(obj){
//                                   obj.destroy();
//                                   battle.scene.unpause();
//                                   finished();
//                               }
//                               );

        launchSeed(curve1);
        launchSeed(curve2);
        launchSeed(curve3);
    }

    function launchSeed(curve) {
        var parent = main.parent;
        var leech = Spawner.spawn(parent.parent, "moving-animated", {
                                   "source": "../../images/leech-seed.gif",
                                   "duration": 750,
                                   "delay": 850,
                                   "pos1": curve.pos1,
                                   "pos2": curve.pos2,
                                   "controlY": curve.controlY,
                                    "z": parent.z
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
