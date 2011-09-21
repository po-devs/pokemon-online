var hailComponent;

function triggerWeather(scene, weather) {
    switch(weather) {
    case FieldData.Rain:
        triggerRain(scene); break;
    case FieldData.Hail:
        triggerHail(scene); break;
    case FieldData.SandStorm:
        triggerSand(scene); break;
    case FieldData.Sunny:
        triggerSun(scene); break;
    }
}

function triggerRain(scene) {
}

function triggerHail(scene) {
    if (!hailComponent) {
        hailComponent = Qt.createComponent("BattleDataQML/Weather/Hail.qml");
    }

    if (hailComponent.status != Component.Ready) {
        return;
    }

    battle.scene.pause();

    var hail = hailComponent.createObject(scene);

    hail.finished.connect(function() {battle.scene.unpause()} )
    hail.start();
}

function triggerSand(scene) {

}

function triggerSun(scene) {

}
