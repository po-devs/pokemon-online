var components = {};

function statUp(pokemon) {

}

function statDown(pokemon) {
    var c;
    if (! ("stat-up" in components) ) {
        var component = Qt.createComponent("CommonEffects/StatDown.qml");

        if (component.status != Component.Ready) {
            console.log("Failed loading components " + "stat-up");
        }

        c = components["stat-up"] = component;
    } else {
        c = components["stat-up"];
    }
    var obj = c.createObject(pokemon, {"pokemon":pokemon});
    obj.finished.connect(function() {obj.destroy();});
    obj.start();
}
