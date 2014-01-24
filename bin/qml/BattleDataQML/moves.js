var components = {};

var effects = {
};

var params = {
};

/* updates the object with properties form other object */
Object.prototype.update = function(other_object) {
    for (var param in other_object) {
        if (other_object.hasOwnProperty(param))
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
effects[BattleData.Boomburst] = effects[BattleData.Earthquake];
effects[BattleData.RapidSpin] = "Moves/RapidSpin.qml";

/* Charging moves */
/* TODO idea: use shader and params to add type color effect to these */
setupWithDefaultParam([BattleData.Tackle, BattleData.TakeDown, BattleData.QuickAttack, BattleData.BodySlam,
    BattleData.Retaliate, BattleData.VoltTackle, BattleData.WildCharge, BattleData.U_turn, BattleData.VoltSwitch,
    BattleData.V_create, BattleData.FlameWheel, BattleData.FlareBlitz, BattleData.FlameCharge,
    BattleData.Rollout, BattleData.IceBall, BattleData.ExtremeSpeed, BattleData.Pursuit, BattleData.PartingShot], "Moves/ChargeMove.qml",
    { attack_time: 300, return_time: 500, rolls: 0, effect: "",
      easing_in_x: Easing.Linear, easing_in_y: Easing.Linear,
      easing_out_x: Easing.Linear, easing_out_y: Easing.Linear}
);

effects[BattleData.MachPunch] = effects[BattleData.BulletPunch] = "Moves/MachPunch.qml";
effects[BattleData.CalmMind] = "Moves/CalmMind.qml";
effects[BattleData.Substitute] = "Moves/Substitute.qml";
effects[BattleData.Bonemerang] = "Moves/Bonemerang.qml";
effects[BattleData.AuraSphere] = "Moves/AuraSphere.qml";

setupWithDefaultParam([BattleData.BoneRush, BattleData.BulletSeed, BattleData.RockBlast], "Moves/BoneRush.qml",
                      { rotations: 2, duration: 500, easing: Easing.Linear
                      });
params[BattleData.BulletSeed].update({rotations: 0, duration: 300, easing: Easing.OutQuad, "image": "../../images/seedx.png"});
params[BattleData.BoneRush].image = "image://pokeinfo/item/200";
params[BattleData.RockBlast].image = "../../images/stealth-rock.png";

effects[BattleData.Detect] = effects[BattleData.Protect] = "Moves/Protect.qml"
effects[BattleData.SpikyShield] = effects[BattleData.KingsShield] = effects[BattleData.Detect]

effects[BattleData.CloseCombat] = "Moves/CloseCombat.qml";
effects[BattleData.Scald] = effects[BattleData.Surf] = "Moves/Surf.qml";

params[BattleData.LeechSeed] = {"image":"leech-seed.gif",
        "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":0, "y":40, "z":0}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":20, "z":0}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":0}, "controlY":70}]};
params[BattleData.StealthRock] = {"image":"stealth-rock.png",
    "curves": [{"pos1":{"x":40, "y":10, "z":0}, "pos2":{"x":-10, "y":40, "z":5}, "controlY":80},
               {"pos1":{"x":30, "y":5, "z":0}, "pos2":{"x":30, "y":80, "z":-8}, "controlY":80},
               {"pos1":{"x":40, "y":15, "z":0}, "pos2":{"x":70, "y":60, "z":5}, "controlY":70}]};
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
params[BattleData.Pursuit].attack_time = 250;
params[BattleData.QuickAttack].attack_time = 200;
params[BattleData.ExtremeSpeed].attack_time = 150;
params[BattleData.U_turn].update({attack_time: 400, easing_in_x: Easing.InQuint, easing_out_x: Easing.InQuint});
params[BattleData.PartingShot] = params[BattleData.VoltSwitch] = params[BattleData.U_turn];
// Placeholder image for FlameWheel
params[BattleData.FlameWheel].update({effect: "../../images/flame0.png", attack_time: 700});
params[BattleData.FlameCharge].update({effect: "../../images/flame0.png", attack_time: 700});
params[BattleData.FlareBlitz].update({effect: "../../images/flame0.png", attack_time: 700, easing_in_x: Easing.InBack,
                                     easing_in_y: Easing.InBack});
params[BattleData.V_create].update({effect: "../../images/flame0.png", attack_time: 700, easing_in_x: Easing.InBack,
                                     easing_in_y: Easing.InBack});
params[BattleData.WildCharge].update({effect: "../../images/lightparticle1.png", attack_time: 700, easing_in_x: Easing.InBack,
                                     easing_in_y: Easing.InBack});
params[BattleData.Rollout].update({rolls: 3, attack_time: 900});
params[BattleData.IceBall].update({rolls: 3, attack_time: 900});

params[BattleData.Protect] = {"color": "#a8a878"};
params[BattleData.Detect] = {"color": "#c03028"};
params[BattleData.SpikyShield] = {"color": "#78c850"};
params[BattleData.KingsShield] = {"color": "#b8b8d0"};
params[BattleData.Scald] = {"flames": true};

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

        if (component.status !== Component.Ready) {
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
