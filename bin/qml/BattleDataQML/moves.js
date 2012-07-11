var components = {};

var effects = {
};

var params = {
};

/* updates the object with properties form other object */
Object.prototype.update = function(other_object) {
    for (var param in other_object) {
        this[param] = other_object[param];
    }
}
/* For faster setup */
function setupWithDefaultParam(moves, move_effect, default_param) {
    moves.forEach(function(move) {
        effects[move] = move_effect;
        params[move] = {};
        params[move].update(default_param);
    });
}

effects[BattleData.LeechSeed] = "Moves/HiddenPebbles.qml";
effects[BattleData.StealthRock] = effects[BattleData.LeechSeed];
effects[BattleData.ToxicSpikes] = effects[BattleData.LeechSeed];
effects[BattleData.Spikes] = effects[BattleData.LeechSeed];

effects[BattleData.Fissure] = effects[BattleData.Earthquake] = "Moves/Earthquake.qml";
effects[BattleData.RapidSpin] = "Moves/RapidSpin.qml";

/* Charging moves */
/* TODO idea: use shader and params to add type color effect to these */
setupWithDefaultParam([BattleData.Tackle, BattleData.TakeDown, BattleData.QuickAttack, BattleData.BodySlam,
    BattleData.Retaliate, BattleData.VoltTackle, BattleData.WildCharge, BattleData.U_turn, BattleData.VoltSwitch,
    BattleData.V_create, BattleData.FlameWheel, BattleData.FlameCharge, BattleData], "Moves/ChargeMove.qml",
    { attack_time: 300, return_time: 500, 
      easing_in_x: Easing.Linear, easing_in_y: Easing.Linear,
      easing_out_x: Easing.Linear, easing_out_y: Easing.Linear}
);

effects[BattleData.Substitute] = "Moves/Substitute.qml";
effects[BattleData.Bonemerang] = "Moves/Bonemerang.qml";
effects[BattleData.RockBlast] = effects[BattleData.BoneRush] = "Moves/BoneRush.qml";

effects[BattleData.Detect] = effects[BattleData.Protect] = "Moves/Protect.qml"


params[BattleData.LeechSeed] = {"image":"leech-seed.gif",
        "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":0, "y":40, "z":0}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":20, "z":0}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":0}, "controlY":70}]};
params[BattleData.StealthRock] = {"image":"stealth-rock.png",
    "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":-10, "y":40, "z":10}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":80, "z":-20}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":10}, "controlY":70}]};
params[BattleData.ToxicSpikes] = {"image":"toxic-spikes.png",
    "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":0, "y":55, "z":0}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":65, "z":0}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":0}, "controlY":70}]};
params[BattleData.Spikes] = {"image":"spikes.png",
    "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":0, "y":55, "z":0}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":65, "z":0}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":0}, "controlY":70}]};

params[BattleData.TakeDown].update({easing_in_x: Easing.OutQuad, easing_in_y: Easing.InQuad});
params[BattleData.BodySlam].easing_in_y = Easing.OutBack;
params[BattleData.QuickAttack].attack_time = 200;
params[BattleData.U_turn].update({attack_time: 400, return_time: 500, easing_in_x: Easing.InQuint, easing_out_x: Easing.InQuint});
params[BattleData.VoltSwitch] = params[BattleData.U_turn];

params[BattleData.Protect] = {"color": "#a8a878"}
params[BattleData.Detect] = {"color": "#c03028"}
params[BattleData.BoneRush] = {"image": "image://pokeinfo/item/200"}
params[BattleData.RockBlast] = {"image": "../../images/stealth-rock.png"}

function useAttack(attacker, attack, defender, params) {
    launchMove(attacker, attack, defender, params);
}

function launchMove(attacker, attack, defender, extras) {
    var key = attack;
    if (! (key in effects)) {
        console.log("Error: Move effect not found: " + key);
        return;
    }

    var c;
    if (! (key in components) ) {
        var component = Qt.createComponent(effects[key]);

        if (component.status != Component.Ready) {
            console.log("Failed loading components " + key + " (" + effects[key] + ")");
            console.log(component.errorString());
            return;
        }

        c = components[key] = component;
    } else {
        c = components[key];
    }

    var p = {"attacker":attacker, "attack":attack, "defender":defender};
    p["extras"] = extras;
    if (key in params) {
        p["params"] = params[key];
    } else {
        p["params"] = {};
    }

    var obj = c.createObject(attacker, p);
    battle.scene.pause();
    obj.finished.connect(function() {battle.scene.unpause();obj.destroy();});
    obj.start();
}
