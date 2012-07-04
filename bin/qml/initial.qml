import QtQuick 1.1

Item
{
    width: 500
    height: 400

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
