var components = {};

var effects = {
};

effects[BattleData.LeechSeed] = "Moves/HiddenPebbles.qml";
effects[BattleData.StealthRock] = effects[BattleData.LeechSeed];
effects[BattleData.ToxicSpikes] = effects[BattleData.LeechSeed];
effects[BattleData.Spikes] = effects[BattleData.LeechSeed];

effects[BattleData.Earthquake] = "Moves/Earthquake.qml";
effects[BattleData.RapidSpin] = "Moves/RapidSpin.qml";
effects[BattleData.TakeDown] = "Moves/TakeDown.qml";
effects[BattleData.QuickAttack] = "Moves/QuickAttack.qml";
effects[BattleData.U_turn] = "Moves/UTurn.qml";
effects[BattleData.Substitute] = "Moves/Substitute.qml";
effects[BattleData.Bonemerang] = "Moves/Bonemerang.qml";
effects[BattleData.BoneRush] = "Moves/BoneRush.qml";

var params = {
};

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
