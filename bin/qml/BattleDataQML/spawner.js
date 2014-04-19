var components = {};

var effects = {
    "moving-animated": "../Utilities/MovingGif.qml"
};

function spawn(parent, type, params, callback) {
    var obj = launchEffect(type, parent, params);

    if (obj) {
        obj.finished.connect(function(){callback(obj)});

        obj.start();
    }
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
    return obj;
}
