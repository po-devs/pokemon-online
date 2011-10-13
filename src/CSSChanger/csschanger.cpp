#include "csschanger.h"
#include "csswidget.h"

ClientPlugin* createPluginClass()
{
    return new CSSPlugin();
}

CSSPlugin::CSSPlugin()
{
}

bool CSSPlugin::hasConfigurationWidget() const
{
    return true;
}

QString CSSPlugin::pluginName() const
{
    return "Theme color changer";
}

QWidget *CSSPlugin::getConfigurationWidget()
{
    return new CssWidget();
}
