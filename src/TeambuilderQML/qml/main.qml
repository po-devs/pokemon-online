import QtQuick 2.2
import PokemonOnlineQml 1.0
import "js/units.js" as U
import "components"
import "ui"
Item {
    id: pokemonOnlineQml
    width: U.dp(8)
    height: U.dp(6)

    state: "serverList"
    Loader {
        id: pageContentLoader
        sourceComponent: serverListPageComponent
        anchors.fill: parent
    }

    Loader {
        id: teamBuilderLoader
        sourceComponent: pokemonOnlineQml.state == "buildTeam" ? buildTeamPageComponent : null
        anchors.fill: parent
    }

    Component {
        id: serverListPageComponent
        ServerListPage {
            onBuildTeamClicked: pokemonOnlineQml.state = "buildTeam";
        }
    }

    Component {
        id: buildTeamPageComponent
        BuildTeamPage {
            onGoBack: pokemonOnlineQml.state = "serverList";
        }
    }
}
