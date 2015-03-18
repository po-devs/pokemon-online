import QtQuick 2.0
import "../components"
import PokemonOnlineQml 1.0
import "../js/units.js" as U

Rectangle {
    signal goBack;
    anchors.fill: parent

    Component.onCompleted: analyserAccess.connectTo("188.165.244.152", 5080)
    VisualDataModel {
        id: visualModel
        model: analyserAccess.playersInfoListModel
        groups: [
            VisualDataGroup {
                id: selectedGroup
                name: "selected"
            }
        ]
        delegate: Rectangle {
            id: item
            height: name == "poqmtest" ? 25 : 0
            width: 200
            clip: true
            Text {
                text: {
                    var text = "Name: " + name
                    if (item.VisualDataModel.inSelected)
                        text += " (" + item.VisualDataModel.selectedIndex + ")"
                    return text;
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!item.VisualDataModel.inSelected)
                        selectedGroup.remove(0, selectedGroup.count)
                    item.VisualDataModel.inSelected = !item.VisualDataModel.inSelected
                }
            }
        }
    }
    Column {
        width: parent.width
        Text {
            text: "Online players"
        }
        Button {
            text: "Back"
            onTriggered: goBack();
        }
        ListView {
            width: parent.width
            height: U.dp(4)
            model: visualModel
        }
        Button {
            id: challengeButton
            text: "Challenge"
            onTriggered:  {
                text = "Challenging"
                analyserAccess.challengeDeclined.connect(function (){
                    challengeButton.text = "Challenge";
                });

                analyserAccess.sendChallenge(selectedGroup.get(0).model.playerId)
            }
        }
    }
}
