#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include "pluginmanager.h"
#include "plugininterface.h"
#include "server.h"
#include "../Utilities/CrossDynamicLib.h"

PluginManager::PluginManager(Server *ser) : server(ser)
{
    QSettings s("config", QSettings::IniFormat);

    QStringList plugins = s.value("plugins").toStringList();
    plugins = plugins.toSet().toList(); /* Remove duplicates */

    foreach(QString plugin, plugins) {
        cross::DynamicLibrary *l;
        try {
             l = new cross::DynamicLibrary(plugin.toLocal8Bit().constData());
        } catch (const std::exception &e) {
            qDebug() << "Error when loading plugin " << plugin <<  ": " << e.what();
            continue;
        }

        libraries.push_back(l);
        PluginInstanceFunction f = (PluginInstanceFunction) l->GetFunction("createPluginClass");

        if (!f) {
            delete l;
            libraries.pop_back();
            continue;
        }

        ServerPlugin *s = f(server);

        if (!s) {
            delete l;
            libraries.pop_back();
            continue;
        }

        this->plugins.push_back(s);
        filenames.push_back(plugin);
    }
}

PluginManager::~PluginManager()
{
    foreach(ServerPlugin *s, plugins) {
        delete s;
    }
    plugins.clear();
    foreach(cross::DynamicLibrary *l, libraries) {
        delete l;
    }
    libraries.clear();
    filenames.clear();
}

void PluginManager::addPlugin(const QString &path)
{
    cross::DynamicLibrary *l;
    try {
         l = new cross::DynamicLibrary(path.toLocal8Bit().constData());
    } catch (const std::exception &e) {
        Server::print("Error when loading plugin " + path +  ": " + e.what());
        return;
    }

    libraries.push_back(l);
    PluginInstanceFunction f = (PluginInstanceFunction) l->GetFunction("createPluginClass");

    if (!f) {
        Server::print("Error when loading plugin " + path +  ": Not a Pokemon Online Server plugin.");
        delete l;
        libraries.pop_back();
        return;
    }

    ServerPlugin *s = f(server);

    if (!s) {
        delete l;
        libraries.pop_back();
        return;
    }

    this->plugins.push_back(s);
    filenames.push_back(path);

    updateSavedList();
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

void PluginManager::updateSavedList()
{
    QSettings s("config", QSettings::IniFormat);
    s.setValue("plugins", filenames);
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

QList<BattlePlugin*> PluginManager::getBattlePlugins(BattleInterface *b)
{
    cleanPlugins();

    QList<BattlePlugin*> ret;

    for (int i =0 ; i < plugins.size(); i++) {
        BattlePlugin *p = plugins[i]->getBattlePlugin(b);

        if (p) {
            ret.push_back(p);
        }
    }

    return ret;
}

ServerPlugin * PluginManager::plugin(const QString &name) const
{
    for (int i = 0; i < plugins.size(); i++) {
        if (plugins[i]->pluginName() == name)
            return plugins[i];
    }

    return NULL;
}

PluginManagerWidget::PluginManagerWidget(PluginManager &pl)
    : pl(pl)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *v = new QVBoxLayout(this);

    v->addWidget(list = new QListWidget());

    list->addItems(pl.getPlugins());

    QHBoxLayout *buttons = new QHBoxLayout();

    v->addLayout(buttons);

    QPushButton *add, *remove;

    buttons->addWidget(add = new QPushButton(tr("Add Plugin...")));
    buttons->addWidget(remove = new QPushButton(tr("Remove Plugin")));

    connect(add, SIGNAL(clicked()), SLOT(addClicked()));
    connect(remove, SIGNAL(clicked()), SLOT(removePlugin()));
}

void PluginManagerWidget::addClicked()
{
    QFileDialog *fd = new QFileDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose, true);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setDirectory("serverplugins");
    fd->show();

    connect(fd, SIGNAL(fileSelected(QString)), this, SLOT(addPlugin(QString)));
    connect(fd, SIGNAL(fileSelected(QString)), fd, SLOT(close()));
}

void PluginManagerWidget::removePlugin()
{
    int row = list->currentRow();

    if (row != -1) {
        pl.freePlugin(row);
        list->clear();
        list->addItems(pl.getPlugins());

        emit pluginListChanged();
    }
}

void PluginManagerWidget::addPlugin(const QString &filename)
{
    QDir d;
    QString rel = d.relativeFilePath(filename);

    pl.addPlugin(rel);

    list->clear();
    list->addItems(pl.getPlugins());

    emit pluginListChanged();
}
