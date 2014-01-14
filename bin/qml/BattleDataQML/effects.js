var components = {};

var effects = {
    "stat-down": "CommonEffects/StatDown.qml",
    "stat-up": "CommonEffects/StatUp.qml"
};

function statUp(pokemon, level) {
    launchEffect("stat-up", pokemon, {"pokemon":pokemon, "level":(level||1)});
}

function statDown(pokemon, level) {
    launchEffect("stat-down", pokemon, {"pokemon":pokemon, "level":(level||1)});
}

function launchEffect(key, parent, vars) {
    if (! (key in effects)) {
        console.log("Error: effect not found: " + key);
        return;
    }

    var c;
    if (! (key in components) ) {
        var component = Qt.createComponent(effects[key]);

        if (component.status !== Component.Ready) {
            console.log("Failed loading components " + key + " (" + effects[key] + ")");
            return;
        }

        c = components[key] = component;
    } else {
        c = components[key];
    }
    var obj = c.createObject(parent, vars);
    obj.finished.connect(function() {obj.destroy();});
    obj.start();
}
