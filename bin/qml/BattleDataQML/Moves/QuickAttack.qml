import QtQuick 1.1
import "../" 1.0

Move {
		id: main;

		property int xt: defender.x-attacker.x;
		property int yt: attacker.y-defender.y;

		property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
		property int y0: attacker.pokeSprite.anchors.bottomMargin;

		SequentialAnimation  {
				id: animation;
				ParallelAnimation {
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: 200; }
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: 200; }
				}
				ParallelAnimation {
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: 500 }
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: 500 }
				}

				ScriptAction { script: finished(); }
		}

		function start() {
				animation.running = true;
		}
}
