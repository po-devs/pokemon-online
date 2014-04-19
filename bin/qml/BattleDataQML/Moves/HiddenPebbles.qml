import QtQuick 2.0
import "../" 1.0
import "../utilities.js" as Utilities
import "../spawner.js" as Spawner

Move {
    id: main;

    /* Available variables:
      - attacker (FieldPokemon)
      - defender (FieldPokemon)
      - attack (int)
      - params (misc)
      */

    Timer {
        id: timer2;
        interval: 300;
    }


    Timer {
        id: timer3;
        interval: 600;
    }

    function adjustCurve(curve) {
        curve.pos1.x += attacker.x;
        curve.pos1.y += attacker.y;
        curve.pos1.z += attacker.z;
        curve.pos2.x += defender.x;
        curve.pos2.y += defender.y;
        curve.pos2.z += defender.z;

        return curve;
    }

    function start() {
        var curve1 = adjustCurve(params.curves[0]);
        var curve2 = adjustCurve(params.curves[1]);
        var curve3 = adjustCurve(params.curves[2]);

        launchPebble(curve1, false);

        timer2.triggered.connect(function(){launchPebble(curve2, false)});
        timer3.triggered.connect(function(){launchPebble(curve3, true)});

        timer2.start();
        timer3.start();
    }

    function launchPebble(curve, finish) {
        var parent = main.parent;
        var leech = Spawner.spawn(parent.parent, "moving-animated", {
                                      "source": "../../images/" + params.image,
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
}
