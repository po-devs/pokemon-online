import QtQuick 1.1

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
        target: battle.scene
        onLaunched: {
            loader.source = "battlescene.qml"
        }
    }
}
