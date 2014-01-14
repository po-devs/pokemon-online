#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QFileInfoList>
#include <QVector>
#include <QStringList>
#include <QSettings>

class Plugin;

namespace cross {
    class DynamicLibrary;
}

class PluginManager
{
    friend class PluginManagerWidget;
public:
    virtual ~PluginManager();

    /* Load plugins for the first time */
    void loadPlugins();

    QStringList getPlugins() const;
    QStringList getVisiblePlugins() const;
    Plugin *plugin(const QString &name) const;
    Plugin *plugin(int index) const;
    /* Number of plugins */
    int count() const;

    //throws std::runtime_error on error
    virtual void addPlugin(const QString &path, QString *name = nullptr);
    virtual void freePlugin(int index);
    //returns true if plugin was found
    bool freePlugin(const QString &name);

    void cleanPlugins();
    /* List of file paths corresponding to possible plugins for the exe */
    QFileInfoList matchingFilePaths() const;
    /* Association of plugin name / plugin path for plugins of the exe */
    QMap<QString, QString> availablePlugins() const;

    /* Returns plugin name if success, empty string otherwise */
    QString testLoad(const QString &filepath) const;
protected:
    /* What settings file to use? */
    virtual QSettings &settings() = 0;
    /* How to instanciate the plugin? */
    virtual Plugin* instanciatePlugin(void *function);
    /* What is the name of the function in the library to create the plugin? */
    virtual const char* instantiatingFunctionName() const {return "createPlugin";}
    /* The version the plugins need to have */
    virtual int version() const = 0;
    /* The directory in which plugins are located by default */
    virtual QString directory() const = 0;
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<Plugin *> plugins;
    QStringList filenames;

    QVector<cross::DynamicLibrary *> lToGo;
    QVector<Plugin *> pToGo;

    void updateSavedList();
};

#endif // PLUGINMANAGER_H
