#ifndef PLUGINMANAGERWIDGET_H
#define PLUGINMANAGERWIDGET_H

#include <QWidget>
#include <QListWidget>

class PluginManager;

class PluginManagerWidget : public QWidget
{
    Q_OBJECT
public:
    PluginManagerWidget(PluginManager &pl);
signals:
    void pluginListChanged();
    void error(QString);
private slots:
    void addClicked();
    void addPlugin(const QString &filename);
    void removePlugin();
private:
    PluginManager &pl;

    QListWidget *list;
    QString defaultFolder;
};

#endif // PLUGINMANAGERWIDGET_H
