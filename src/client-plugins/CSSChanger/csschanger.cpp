#include "csschanger.h"
#include "csswidget.h"
#include "../Teambuilder/engineinterface.h"

ClientPlugin* createPluginClass(MainEngineInterface*i)
{
    return new CSSPlugin(i->theme());
}

CSSPlugin::CSSPlugin(ThemeAccessor*theme) : theme(theme)
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
    return new CssWidget(theme);
}
