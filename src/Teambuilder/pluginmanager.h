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
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<ClientPlugin *> plugins;
    QHash<ClientInterface*, QHash<ClientPlugin *, OnlineClientPlugin*> > clientPlugins;
    QStringList filenames;

    void updateSavedList();
    MainEngine *engine;
    QSet<ClientInterface *> clients;
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
