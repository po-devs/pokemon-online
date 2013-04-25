#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>
#include <QWidget>
#include <QListWidget>

class PlayerInterface;
class ServerPlugin;
class ChallengeInfo;
class BattlePlugin;
class BattleInterface;
class Server;

namespace cross {
    class DynamicLibrary;
};

class PluginManager
{
    friend class PluginManagerWidget;
public:
    PluginManager(Server *s);
    ~PluginManager();

    QStringList getPlugins() const;
    QStringList getVisiblePlugins() const;
    QList<BattlePlugin*> getBattlePlugins(BattleInterface *);

    ServerPlugin *plugin(const QString &name) const;

    void addPlugin(const QString &path);
    void freePlugin(int index);

    void cleanPlugins();
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<ServerPlugin *> plugins;
    QStringList filenames;
    Server *server;

    QVector<cross::DynamicLibrary *> lToGo;
    QVector<ServerPlugin *> pToGo;

    void updateSavedList();
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
