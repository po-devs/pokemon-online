/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project on Qt Labs.
**
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions contained
** in the Technology Preview License Agreement accompanying this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
****************************************************************************/

import QtQuick 1.1

TopLevelItem {
    id: logger;
    property string text;
    property bool shown: false;

    width: model.width + 22;
    height: model.height + background.border.top + background.border.bottom;

    opacity: logger.text !== "";
    visible: logger.shown;

    // Visible items bellow should anchor / be sized in relation to
    // 'parent'. They'll be reparented to a proper positioned and
    // resized toplevel item.
    keepInside: true;

    Behavior on height {
        NumberAnimation {duration:300;}
    }

    Behavior on opacity {
        NumberAnimation {duration: 300;}
    }

    BorderImage {
        id: background;
        anchors.fill: parent;
        source: "../../images/tooltip-background.png";
        border.top: 5;
        border.left: 11;
        border.bottom: 2;
        border.right: 11;
    }

    // ### This Text is used to get the "preferred size" information, that
    // will be considered when calculating the toplevel item geometry. This
    // could be replaced by having this information available in a regular Text
    // item. Similar issue of trying to know the "real image size" inside an
    // Image item.
    Text {
        id: model;
        text: logger.text;
        visible: false;
    }

    Text {
        anchors.centerIn: parent;
        anchors.verticalCenterOffset: 1;
        width: parent.width - 22;
        horizontalAlignment: Text.AlignHCenter;

        text: logger.text;
        color: "#ffffff";
        elide: Text.ElideRight
    }

    Timer {
        id: clearer;
        interval: 750;
        repeat: false;
        onTriggered: {logger.text = ""; if (unpause) {unpause=false; battle.scene.unpause();}}

        property bool unpause: false;
    }

    SequentialAnimation {
        id: delayer;

        ScriptAction {
            script: {battle.scene.pause();}
        }

        PauseAnimation { duration: 280 }

        ScriptAction {
            script: {battle.scene.unpause();}
        }
    }

    function log(text) {
        if (text === "") {
            if (!clearer.unpause) {
                battle.scene.pause();
            }
            clearer.unpause = true;
            clearer.restart();
            return;
        }

        logger.text += text;

        /* Gives more time before the text is deleted */
        clearer.restart();

        if (!delayer.running) {
            delayer.start();
        }
    }
}
