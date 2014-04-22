import QtQuick 2.0

Item
{
    width: screenSize().w;
    height: screenSize().h

    function screenSize() {
        var str = battle.scene.option("screensize", "500x400");
        return {"w": str.substr(0, str.indexOf('x')), "h":str.substr(str.indexOf('x')+1)};
    }

    Loader {
        id: loader
    }

    Connections {
        ignoreUnknownSignals: true
        target: battle.scene
        onLaunched: {
            console.log("Switching to the scene")
            loader.source = "battlescene.qml"
            console.log("Switched to the scene")
        }
    }
}
