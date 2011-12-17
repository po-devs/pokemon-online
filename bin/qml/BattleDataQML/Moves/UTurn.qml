import QtQuick 1.0
import "../" 1.0

Move {
		id: main;

		property int xt: target.x-attacker.x;
		property int yt: attacker.y-target.y;

		property int x0: attacker.pokeSprite.anchors.horizontalCenterOffset;
		property int y0: attacker.pokeSprite.anchors.bottomMargin;

		/*
		 * The main animation of Rapid Spin
		 */
		SequentialAnimation  {
				id: animation;
				ParallelAnimation {
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: xt; duration: 400; easing.type: Easing.InQuint }
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: yt; duration: 400 }
				}
				ParallelAnimation {
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.horizontalCenterOffset"; to: x0; duration: 500; easing.type: Easing.InQuint }
					NumberAnimation { target: attacker.pokeSprite; property: "anchors.bottomMargin"; to: y0; duration: 500 }
				}
				// Delete the Move object
				ScriptAction { script: finished(); }
		}

		function start() {
				animation.running = true;
		}
}
