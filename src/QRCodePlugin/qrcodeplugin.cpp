#include "qrcodeplugin.h"
#include "../Teambuilder/engineinterface.h"
#include "../Teambuilder/Teambuilder/teamholderinterface.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/otherwidgets.h"
#ifdef _WIN32
#include "../../SpecialIncludes/zlib.h"
#include "../../SpecialIncludes/qrencode.h"
#else
#include <zlib.h>
#include <qrencode.h>
#endif
#include <QLabel>
#include <QBitmap>
#include <QPalette>

ClientPlugin* createPluginClass(MainEngineInterface *_interface)
{
    return new QRCodePlugin(_interface);
}

QRCodePlugin::QRCodePlugin(MainEngineInterface *_interface) : _interface(_interface)
{
}

bool QRCodePlugin::hasConfigurationWidget() const
{
    return true;
}

QString QRCodePlugin::pluginName() const
{
    return "Export team to QRCode";
}

QWidget *QRCodePlugin::getConfigurationWidget()
{
    TeamHolderInterface *team = _interface->trainerTeam();
    QByteArray xml = team->team().toXml().toUtf8();

    /* Creates zipped data */
    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;

    deflateInit(&stream, Z_BEST_COMPRESSION);

    int mem = deflateBound(&stream, xml.length());

    uchar *bufptr = (uchar*)malloc(mem);

    stream.next_out = bufptr;
    stream.avail_out = mem;
    stream.next_in = (Bytef*)xml.constData();
    stream.avail_in = xml.length();

    int res = deflate(&stream, Z_FINISH);

    if (res != Z_STREAM_END) {
        free(bufptr);

        /* Error while zipping */
        return new QLabel(QObject::tr("Error while loading the widget, zipping of the team failed: %1").arg(stream.msg));
    }

    int length = stream.total_out;

    QRinput *input = QRinput_new();
    QRinput_append(input, QR_MODE_8, length, bufptr);
    QRcode *code = QRcode_encodeInput(input);

    QRinput_free(input);
    free(bufptr);

    int outputlen = (code->width+7)/8*code->width;
    uchar output[outputlen];
    memset(output, 0, outputlen);

    uchar *iterator = output;
    uchar *initerator = code->data;
    int cptr = 0;

    /* Converts to bitmap format */
    for (int i = 0; i < code->width; i++) {
        for (int j = 0; j < code->width; j++) {

            *iterator |= (*initerator&0x1) << cptr;
            initerator++;

            cptr++;
            if (cptr >= 8) {
                cptr = 0;
                iterator++;
            }
        }

        if (cptr != 0) {
            cptr = 0;
            iterator++;
        }
    }

    QBitmap bitmap = QBitmap::fromData(QSize(code->width, code->width), output);
    bitmap = bitmap.transformed(QTransform::fromScale(5,5));

    QRcode_free(code);

    QLabel *ret = new QDraggableLabel();
    QPalette palette = ret->palette();
    palette.setColor(QPalette::Foreground, Qt::black);
    palette.setColor(QPalette::Background, Qt::white);
    ret->setPalette(palette);
    ret->setPixmap(bitmap);

    ret->setMargin(20);
    return ret;
}
