#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>
#include <QtGui>
#ifdef QT5
#include <QtWidgets>
#endif

class ClientPlugin;
class MainEngine;
class ClientInterface;
class OnlineClientPlugin;
class TeambuilderInterface;
class TeambuilderPlugin;

namespace cross {
    class DynamicLibrary;
}

class PluginManager
{
    friend class PluginManagerWidget;
public:
    PluginManager(MainEngine *t);
    ~PluginManager();

    QStringList getPlugins() const;
    QStringList getVisiblePlugins() const;
    ClientPlugin *plugin(const QString &name) const;

    void addPlugin(const QString &path);
    void freePlugin(int index);

    void launchClient(ClientInterface *c);
    void quitClient(ClientInterface *c);

    void launchTeambuilder(TeambuilderInterface *c);
    void quitTeambuilder(TeambuilderInterface *c);
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<ClientPlugin *> plugins;
    QHash<ClientInterface*, QHash<ClientPlugin *, OnlineClientPlugin*> > clientPlugins;
    QHash<TeambuilderInterface*, QHash<ClientPlugin *, TeambuilderPlugin*> > teambuilderPlugins;
    QStringList filenames;

    void updateSavedList();
    MainEngine *engine;
    QSet<ClientInterface *> clients;
    QSet<TeambuilderInterface *> teambuilders;
};

class PluginManagerWidget : public QWidget
{
    Q_OBJECT
public:
    PluginManagerWidget(PluginManager &pl);

signals:
    void pluginListChanged();
private slots:
    void addClicked();
    void addPlugin(const QString &filename);
    void removePlugin();
private:
    PluginManager &pl;

    QListWidget *list;
};

#endif // PLUGINMANAGER_H
