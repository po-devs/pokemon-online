var sources = {};

sources[FieldData.Rain] = "BattleDataQML/Weather/Rain.qml";
sources[FieldData.Hail] = "BattleDataQML/Weather/Hail.qml";
sources[FieldData.SandStorm] = "BattleDataQML/Weather/Sand.qml";
sources[FieldData.Sunny] = "BattleDataQML/Weather/Sun.qml";

function trigger(weatherScene, weather) {
    //todo
    return;
    if (weather in sources) {
        weatherScene.source = sources[weather];
        weatherScene.item.parent = weatherScene.parent;
        weatherScene.item.start();
    }
}
