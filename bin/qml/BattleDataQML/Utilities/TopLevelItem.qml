/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project on Trolltech Labs.
**
** $QT_BEGIN_LICENSE:LGPL-ONLY$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.0

Item {
    id: placeholder;
    default property alias data: topLevelItem.data;
    property alias transformOrigin: topLevelItem.transformOrigin;
    property alias scale: topLevelItem.scale;
    property alias rotation: topLevelItem.rotation;

    // If true, the toplevel item will be constrained to be inside the
    // topLevelParent.
    property bool keepInside: false;

    Item {
        id: topLevelItem;
        parent: topLevelParent(placeholder);
        x: mappedX(placeholder);
        y: mappedY(placeholder);
        width: placeholder.width;
        height: placeholder.height;

//        onXChanged: {
//            console.log("Dimensions: x,y,w,h: " + x + " " + y + " " + width + " " + height)
//        }

        opacity: placeholder.opacity;
        visible: placeholder.visible;

        function bound(min, value, max) {
            return Math.min(Math.max(min, value), max);
        }

        states: [
            State {
                name: "keepInside";
                when: placeholder.keepInside;
                PropertyChanges {
                    target: topLevelItem;
                    x: bound(0, mappedX(placeholder), parent.width - width);
                    y: bound(0, mappedY(placeholder), parent.height - height);
                    width: Math.min(parent.width, placeholder.width);
                    height: Math.min(parent.height, placeholder.height);
                }
            }
        ]
    }

    function topLevelParent(item)
    {
        while (item.parent !== null)
            item = item.parent;
        return item;
    }

    function mappedX(item)
    {
        var x = item.x;
        while (item.parent !== null) {
            item = item.parent;
            x += item.x;
        }
        return x;
    }
    function mappedY(item)
    {
        var y = item.y;
        while (item.parent !== null) {
            item = item.parent;
            y += item.y;
        }
        return y;
    }
}
