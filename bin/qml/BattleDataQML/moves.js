var components = {};

var effects = {
};

effects[BattleData.LeechSeed] = "Moves/LeechSeed.qml";

function useAttack(attacker, attack, defender) {

}

function launchMove(attacker, attack, defender) {
    if (! (key in effects)) {
        console.log("Error: effect not found: " + key);
        return;
    }

    var c;
    if (! (key in components) ) {
        var component = Qt.createComponent(effects[key]);

        if (component.status != Component.Ready) {
            console.log("Failed loading components " + key + " (" + effects[key] + ")");
            return;
        }

        c = components[key] = component;
    } else {
        c = components[key];
    }

    var obj = c.createObject(attacker, {"attacker":attacker, "attack":attack, "target":defender});
    obj.finished.connect(function() {obj.destroy();});
    obj.start();
}
