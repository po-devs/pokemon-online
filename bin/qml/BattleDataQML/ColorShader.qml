import QtQuick 1.1
import Qt.labs.shaders 1.0

/**
 *Blends a color on a picture, following its mask if any.
 *
 *
 * Usage : ColorShader {sourceImage: widget, blendColor: color, enabled: true}
 *
 * Optional parameters is alpha, which is defaulted to 0.5
 * Call grab() everytime you update your image. If you have
 * "live images", i.e. animations, then maybe expose the live
 * property, otherwise do shader.sourceShader.live = true
*/
ShaderEffectItem {
    property variant image
    property real alpha: 0.5

    ShaderEffectSource {
        id: sourceImage
        hideSource: false
        sourceItem: image
        live: false
    }

    function grab() {
        sourceImage.grab();
    }

    x: image.x
    y: image.y
    z: image.z + 1
    width: image.width
    height: image.height
    parent: image.parent
    scale: image.scale
    rotation: image.rotation
    transformOrigin: image.transformOrigin

    property real opac: image.opacity

    property variant sourceTexture: sourceImage

    property color blendColor: "black"


    vertexShader: "
            uniform lowp float rz2;
            uniform highp mat4 qt_ModelViewProjectionMatrix;
            attribute highp vec4 qt_Vertex;
            attribute highp vec2 qt_MultiTexCoord0;

            varying highp vec2 qt_TexCoord;

            void main(void)
            {
                qt_TexCoord = qt_MultiTexCoord0;
                gl_Position =  qt_ModelViewProjectionMatrix * qt_Vertex;
            }
        "

    fragmentShader:
            "uniform highp sampler2D sourceTexture;
            uniform lowp vec4 blendColor;
            uniform lowp float alpha;
            uniform lowp float opac;
            varying highp vec2 qt_TexCoord;

            void main (void)
            {
                highp vec4 c = texture2D(sourceTexture, qt_TexCoord);

                gl_FragColor = vec4((c.rgb*(1.0-alpha)+blendColor.rgb*alpha)*c.a*opac, c.a*opac);
            }"
}
