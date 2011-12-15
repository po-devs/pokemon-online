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

    Timer {
        id: timer2;
        interval: 300;
    }


    Timer {
        id: timer3;
        interval: 600;
    }

    function start() {
        var curve1 = {"pos1":{"x":attacker.x+40, "y":attacker.y+10}, "pos2":{"x":target.x, "y":target.y+40}, "controlY":80};
        var curve2 = {"pos1":{"x":attacker.x+30, "y":attacker.y+5}, "pos2":{"x":target.x+30, "y":target.y+20}, "controlY":80};
        var curve3 = {"pos1":{"x":attacker.x+40, "y":attacker.y+15}, "pos2":{"x":target.x+70, "y":target.y+60}, "controlY":70};

        launchSeed(curve1, false);

        timer2.triggered.connect(function(){launchSeed(curve2, false)});
        timer3.triggered.connect(function(){launchSeed(curve3, true)});

        timer2.start();
        timer3.start();
    }

    function launchSeed(curve, finish) {
        var parent = main.parent;
        var leech = Spawner.spawn(parent.parent, "moving-animated", {
                                      "source": "../../images/leech-seed.gif",
                                      "duration": 750,
                                      "delay": 1000,
                                      "curve": curve,
                                      "z": parent.z
                                  },
                                  function(obj){
                                      finished.connect(function(){obj.destroy()});
                                      if (finish) {
                                          finished();
                                      }
                                  }
                                  );
    }

    signal finished();
}
