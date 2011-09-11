import QtQuick 1.0

Rectangle {
    id: rectangle1
    width: 500
    height: 400
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#cfa50d"
        }

        GradientStop {
            position: 0.460
            color: "#9e7373"
        }

        GradientStop {
            position: 0.700
            color: "#7794a6"
        }

        GradientStop {
            position: 1
            color: "#40598d"
        }
    }


}
