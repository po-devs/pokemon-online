var components = {};

var effects = {
    "curve": "import QtQuick 1.1; Item{property point pos1; property point pos2; property int controlY}"
};

/* Makes a curve, given pos of origin and destination,
  plus Y amplitude.

  Coordinates can be accessed using ret.x(percent) and ret.y(percent) */
function makeCurve(pos1, pos2, controlY) {
    var ret = function() {
        this.x = function(percent) {
            console.log("calling x");
            return pos1.x + (pos2.x-pos1.x)*percent;
        };

        this.y = function(percent) {
            var baseY = pos1.y + (pos2.y-pos1.y)*percent;
            var addY = controlY * (1 - (0.5-percent)*(0.5-percent)*4);

            console.log("calling y");
            return baseY + addY;
        };
    }

    return ret;
}

function typeImg(type) {
    return "<img src='../../../Themes/Classic/types/type" + type + ".png' />";
}
