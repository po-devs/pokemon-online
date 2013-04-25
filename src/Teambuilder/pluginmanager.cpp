#include "pluginmanager.h"
#include "plugininterface.h"
#include "mainwindow.h"
#include "clientinterface.h"
#include "teambuilderinterface.h"
#include "../Utilities/CrossDynamicLib.h"

PluginManager::PluginManager(MainEngine *t) : engine(t)
{
    QSettings s;

    QStringList plugins = s.value("plugins").toStringList();
    plugins = plugins.toSet().toList(); /* Remove duplicates */

    foreach(QString plugin, plugins) {
        cross::DynamicLibrary *l;
        try {
#ifdef QT5
             l = new cross::DynamicLibrary(plugin.toLatin1().constData());
#else
             l = new cross::DynamicLibrary(plugin.toAscii().constData());
#endif
        } catch (const std::exception &e) {
            qDebug() << "Error when loading plugin " << plugin << ": " << e.what();
            continue;
        }

        libraries.push_back(l);
        PluginInstanceFunction f = (PluginInstanceFunction) l->GetFunction("createPluginClass");

        if (!f) {
            delete l;
            libraries.pop_back();
            continue;
        }

        ClientPlugin *s = f(engine);

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
    foreach(ClientPlugin *s, plugins) {
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
#ifdef QT5
         l = new cross::DynamicLibrary(path.toLatin1().constData());
#else
         l = new cross::DynamicLibrary(path.toAscii().constData());
#endif
    } catch (const std::exception &e) {
        QMessageBox::warning(NULL, QObject::tr("Pokemon Online"), QObject::tr("Error when loading plugin at %1: %2").arg(path).arg(e.what()));
        return;
    }

    libraries.push_back(l);
    PluginInstanceFunction f = (PluginInstanceFunction) l->GetFunction("createPluginClass");

    if (!f) {
        QMessageBox::warning(NULL, QObject::tr("Pokemon Online"), QObject::tr("%1 is not a Pokemon Online plugin.").arg(path));
        delete l;
        libraries.pop_back();
        return;
    }

    ClientPlugin *s = f(engine);

    if (!s) {
        delete l;
        libraries.pop_back();
        return;
    }

    foreach (ClientInterface *ci, clients) {
        OnlineClientPlugin *ocp = s->getOnlinePlugin(ci);

        if (ocp) {
            clientPlugins[ci].insert(s, ocp);
            ci->addPlugin(ocp);
        }
    }
    foreach (TeambuilderInterface *ci, teambuilders) {
        TeambuilderPlugin *ocp = s->getTeambuilderPlugin(ci);

        if (ocp) {
            teambuilderPlugins[ci].insert(s, ocp);
            ci->addPlugin(ocp);
        }
    }

    this->plugins.push_back(s);
    filenames.push_back(path);

    updateSavedList();
}

void PluginManager::freePlugin(int index)
{
    if (index < plugins.size() && index >= 0) {
        ClientPlugin *p = plugins[index];
        foreach(ClientInterface *ci, clients) {
            if (clientPlugins.value(ci).contains(p)) {
                ci->removePlugin(clientPlugins[ci][p]);
                delete clientPlugins[ci][p];
                clientPlugins[ci].remove(p);
            }
        }
        foreach(TeambuilderInterface *ci, teambuilders) {
            if (teambuilderPlugins.value(ci).contains(p)) {
                ci->removePlugin(teambuilderPlugins[ci][p]);
                delete teambuilderPlugins[ci][p];
                teambuilderPlugins[ci].remove(p);
            }
        }

        delete plugins[index];
        delete libraries[index];
        plugins.erase(plugins.begin() + index, plugins.begin() + index + 1);
        libraries.erase(libraries.begin() + index, libraries.begin() + index + 1);
        filenames.erase(filenames.begin() + index, filenames.begin() + index + 1);

        updateSavedList();
    }
}

void PluginManager::updateSavedList()
{
    QSettings s;
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

ClientPlugin * PluginManager::plugin(const QString &name) const
{
    for (int i = 0; i < plugins.size(); i++) {
        if (plugins[i]->pluginName() == name)
            return plugins[i];
    }

    return NULL;
}

void PluginManager::launchClient(ClientInterface *c)
{
    clients.insert(c);

    foreach(ClientPlugin *pl, plugins) {
        OnlineClientPlugin *o = pl->getOnlinePlugin(c);

        if (o) {
            c->addPlugin(o);
            clientPlugins[c].insert(pl, o);
        }
    }
}

void PluginManager::quitClient(ClientInterface *c)
{
    foreach(OnlineClientPlugin *o, clientPlugins.value(c)) {
        delete o;
    }

    clientPlugins.remove(c);
    clients.remove(c);
}

void PluginManager::launchTeambuilder(TeambuilderInterface *c)
{
    teambuilders.insert(c);

    foreach(ClientPlugin *pl, plugins) {
        TeambuilderPlugin *o = pl->getTeambuilderPlugin(c);

        if (o) {
            c->addPlugin(o);
            teambuilderPlugins[c].insert(pl, o);
        }
    }
}

void PluginManager::quitTeambuilder(TeambuilderInterface *c)
{
    foreach(TeambuilderPlugin *o, teambuilderPlugins.value(c)) {
        delete o;
    }

    teambuilderPlugins.remove(c);
    teambuilders.remove(c);
}


/*************************************************************/
/*************************************************************/
/*************************************************************/

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
    fd->setDirectory("myplugins");
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
