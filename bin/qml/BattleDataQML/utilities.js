/* Makes a curve, given pos of origin and destination,
  plus Y amplitude.

  Coordinates can be accessed using ret.x(percent) and ret.y(percent) */
function makeCurve(pos1, pos2, controlY) {
    var ret = function() {
        this.x = function(percent) {
            return pos1.x + (pos2.x-pos1.x)*percent;
        };

        this.y = function(percent) {
            var baseY = pos1.y + (pos2.y-pos1.y)*percent;
            var addY = controlY * (1 - (0.5-percent)*(0.5-percent)*4);

            return baseY + addY;
        };
    }

    return ret;
}
