#include "thememanager.h"
#include "thememanagerwidget.h"

#include <QCoreApplication>

ClientPlugin* createClientPlugin(MainEngineInterface*)
{
    return new ThemeManagerPlugin();
}

ThemeManagerPlugin::ThemeManagerPlugin()
{
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
}

bool ThemeManagerPlugin::hasConfigurationWidget() const
{
    return true;
}

QString ThemeManagerPlugin::pluginName() const
{
    return "Theme Manager";
}

QWidget *ThemeManagerPlugin::getConfigurationWidget()
{
    return new ThemeManagerWidget();
}
