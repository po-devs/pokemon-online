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
    Item {
        id: pageStack
        property var stack: [];
        function push(component) {
            stack.push(component.createObject(pokemonOnlineQml));
        }
        function pop() {
            stack[stack.length - 1].destroy()
            stack.pop();
        }
        Component.onCompleted: pageStack.push(mainMenuPageComponent)
    }


    AnalyzerAccess {
        id: analyserAccess
    }


    Component {
        id: mainMenuPageComponent
        MainMenuPage {
            onBuildTeamClicked: pageStack.push(buildTeamPageComponent);
        }
    }

//    Component {
//        id: serverListPageComponent
//        ServerListPage {
//            onBuildTeamClicked: pokemonOnlineQml.state = "buildTeam";
//        }
//    }

    Component {
        id: buildTeamPageComponent
        BuildTeamPage {
            onGoBack: pageStack.pop()
            onGoFindGame: pageStack.push(findGamePageComponent)
        }
    }

    Component {
        id: findGamePageComponent
        FindGamePage {
            onGoBack: pageStack.pop()
        }
    }
}
