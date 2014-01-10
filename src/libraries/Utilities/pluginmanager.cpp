#include <stdexcept>

#include <QSet>
#include <QDebug>
#include <QDir>

#include "exesuffix.h"
#include "CrossDynamicLib.h"
#include "pluginmanager.h"
#include "plugininterface.h"

void PluginManager::loadPlugins()
{
    QStringList plugins = settings().value("plugins" SUFFIX).toStringList();
    plugins = plugins.toSet().toList(); /* Remove duplicates */

    foreach(QString plugin, plugins) {
        try {
            addPlugin(plugin);
        } catch (const std::runtime_error &err) {
            qDebug() << err.what();
        }
    }
}

PluginManager::~PluginManager()
{
    foreach(Plugin *s, plugins) {
        delete s;
    }
    plugins.clear();
    foreach(cross::DynamicLibrary *l, libraries) {
        delete l;
    }
    libraries.clear();
    filenames.clear();
}

void PluginManager::addPlugin(const QString &path, QString *name)
{
    cross::DynamicLibrary *l;
    try {
         l = new cross::DynamicLibrary(path.toLocal8Bit().constData());
    } catch (const std::exception &e) {
        throw std::runtime_error(QString("Error when loading plugin " + path +  ": " + e.what()).toStdString());
    }

    libraries.push_back(l);
    void* f = l->GetFunction(instantiatingFunctionName());

    if (!f) {
        delete l;
        libraries.pop_back();

        throw std::runtime_error(QString("Error when loading plugin " + path +  ": Not a pokemon online plugin for this program.").toStdString());
    }

    Plugin *s = instanciatePlugin(f);

    if (!s) {
        delete l;
        libraries.pop_back();

        throw std::runtime_error(QString("Error when loading plugin " + path +  ": Not a pokemon online plugin for this program or version outdated.").toStdString());
    }

    if (s->version() != version()) {
        delete l;
        libraries.pop_back();

        throw std::runtime_error(QString("Error when loading plugin " + path +  ": different version than this program.").toStdString());
    }

    if (name) {
        *name = s->pluginName();

        delete l;
        libraries.pop_back();

        return;
    }

    if (getPlugins().indexOf(s->pluginName()) != -1) {
        delete l;
        libraries.pop_back();

        throw std::runtime_error(QString("Error when loading plugin " + path +  ": Plugin with the same name already loaded.").toStdString());
    }

    this->plugins.push_back(s);
    filenames.push_back(path);

    updateSavedList();

    s->init();
}

Plugin* PluginManager::instanciatePlugin(void *function)
{
    return ((PluginInstanceFunction)function)();
}

void PluginManager::freePlugin(int index)
{
    if (index < plugins.size() && index >= 0) {
        if (plugins[index]->isReadyForDeletion()) {
            delete plugins[index];
            delete libraries[index];
        } else {
            lToGo.push_back(libraries[index]);
            pToGo.push_back(plugins[index]);
        }
        plugins.erase(plugins.begin() + index, plugins.begin() + index + 1);
        libraries.erase(libraries.begin() + index, libraries.begin() + index + 1);
        filenames.erase(filenames.begin() + index, filenames.begin() + index + 1);

        updateSavedList();
    }
}

bool PluginManager::freePlugin(const QString &name)
{
    for (int i = 0; i < plugins.size(); i++) {
        if (plugins[i]->pluginName() == name) {
            freePlugin(i);
            return true;
        }
    }

    return false;
}

void PluginManager::cleanPlugins()
{
    for (int i = 0; i < pToGo.size(); i++) {
        if (pToGo[i]->isReadyForDeletion()) {
            delete pToGo[i];
            delete lToGo[i];
            pToGo.erase(pToGo.begin() + i, pToGo.begin() + i + 1);
            lToGo.erase(lToGo.begin() + i, lToGo.begin() + i + 1);
            i--;
        }
    }
}

QFileInfoList PluginManager::matchingFilePaths() const
{
    QDir d;
    d.cd(directory());

    auto files = d.entryInfoList(QStringList() << (QString("*") + OS_LIB_SUFFIX), QDir::Files);

    QFileInfoList resFiles;

    foreach(QFileInfo info, files) {
        if (strlen(SUFFIX) == 0) {
            if (!info.fileName().contains("_debug")) {
                resFiles.push_back(info);
            }
        } else {
            if (info.fileName().contains(SUFFIX)) {
                resFiles.push_back(info);
            }
        }
    }

    return resFiles;
}

QMap<QString,QString> PluginManager::availablePlugins() const
{
    QFileInfoList files = matchingFilePaths();

    QMap<QString,QString> ret;

    foreach(QFileInfo file, files) {
        QString name = testLoad(file.absoluteFilePath());

        ret[file.absoluteFilePath()] = name;
    }

    return ret;
}

QString PluginManager::testLoad(const QString &filepath) const
{
    QString name;

    const_cast<PluginManager*>(this)->addPlugin(filepath, &name);

    return name;
}

void PluginManager::updateSavedList()
{
    settings().setValue("plugins", filenames);
}

QStringList PluginManager::getPlugins() const
{
    QStringList ret;

    for (int i = 0; i < plugins.size(); i++) {
        ret.append(plugins[i]->pluginName());
    }

    return ret;
}

QStringList PluginManager::getVisiblePlugins() const
{
    QStringList ret;

    for (int i = 0; i < plugins.size(); i++) {
        if (plugins[i]->hasConfigurationWidget())
            ret.append(plugins[i]->pluginName());
    }

    return ret;
}

Plugin * PluginManager::plugin(const QString &name) const
{
    for (int i = 0; i < plugins.size(); i++) {
        if (plugins[i]->pluginName() == name)
            return plugins[i];
    }

    return nullptr;
}


Plugin * PluginManager::plugin(int index) const
{
    return plugins[index];
}

int PluginManager::count() const
{
    return plugins.size();
}
