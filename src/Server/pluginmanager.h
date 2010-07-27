#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>
#include <QtGui>
class PlayerInterface;
class ServerPlugin;
class ChallengeInfo;

namespace cross {
    class DynamicLibrary;
};

class PluginManager
{
    friend class PluginManagerWidget;
public:
    PluginManager();
    ~PluginManager();

    void battleStarting(PlayerInterface *p1, PlayerInterface *p2, const ChallengeInfo &c);

    QStringList getPlugins() const;
    QStringList getVisiblePlugins() const;
    ServerPlugin *plugin(const QString &name) const;

    void addPlugin(const QString &path);
    void freePlugin(int index);
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<ServerPlugin *> plugins;
    QStringList filenames;

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
