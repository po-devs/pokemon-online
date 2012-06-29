import QtQuick 1.1

Item {
    property variant pos1;
    property variant pos2;
    property int  controlY;

    function x(percent) {
        return pos1.x + (pos2.x-pos1.x)*percent;
    }

    function y(percent) {
        var baseY = pos1.y + (pos2.y-pos1.y)*percent;
        var addY = controlY * (1 - (0.5-percent)*(0.5-percent)*4);

        return baseY - addY;
    }

    function z(percent) {
        return pos1.z + (pos2.z-pos1.z)*percent;
    }
}
