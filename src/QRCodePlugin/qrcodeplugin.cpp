#include "qrcodeplugin.h"
#include "../Teambuilder/engineinterface.h"
#include "../PokemonInfo/pokemonstructs.h"
#ifdef WIN32
#include "../../SpecialIncludes/zlib.h"
#else
#include <zlib.h>
#endif
#include <QLabel>

ClientPlugin* createPluginClass(MainEngineInterface *interface)
{
    return new QRCodePlugin(interface);
}

QRCodePlugin::QRCodePlugin(MainEngineInterface *interface) : interface(interface)
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
    TrainerTeam *team = interface->trainerTeam();
    QByteArray xml = team->toXml().toUtf8();

    /* Creates zipped data */
    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;

    deflateInit(&stream, Z_BEST_COMPRESSION);

    int mem = deflateBound(&stream, xml.length());

    uchar *bufptr = (uchar*)malloc(mem);

    stream.next_out = (Bytef*)malloc(mem);
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

    free(bufptr);

    return new QLabel(QObject::tr("The zipped team takes %1 bytes.").arg(length));
}
