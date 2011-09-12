import QtQuick 1.0

Rectangle {
    id: rectangle1
    width: 500
    height: 400
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#cfa50d"
        }

        GradientStop {
            position: 0.460
            color: "#9e7373"
        }

        GradientStop {
            position: 0.700
            color: "#7794a6"
        }

        GradientStop {
            position: 1
            color: "#40598d"
        }
    }

    Component {
        id: pokemon

        Item {
            property QtObject poke: QtObject{}
            width: 32
            height: 32

            Image {
                id: pokeicon
                source: "image://pokeinfo/icon/"+index
                width: 32
                height: 32
            }
        }
    }

    Component {
        id: team

        Item {
            property QtObject team: QtObject {}
            anchors.fill: parent

            Grid {
                anchors.fill: parent
                rows: 2
                columns: 3
                spacing: 12
                Repeater {
                    model: 6
                    delegate: pokemon
                }
            }
        }
    }

    Rectangle {
        id: team1
        property int player: 0
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        color:"transparent"
        width: 120
        height: 76
        Loader {
            anchors.fill: parent
            sourceComponent: team
        }
    }

    Rectangle {
        id: team2
        color: "transparent"
        property int player: 1
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        width: 120
        height: 76
        Loader {
            anchors.fill: parent
            sourceComponent: team
        }
    }
}
