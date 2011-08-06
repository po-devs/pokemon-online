#include "qrcodeplugin.h"
#include <QLabel>

ClientPlugin* createPluginClass() {
    return new QRCodePlugin();
}

QRCodePlugin::QRCodePlugin()
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
    return new QLabel("Test Sucessful");
}
