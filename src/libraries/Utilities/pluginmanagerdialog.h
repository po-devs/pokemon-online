#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H

#include <QDialog>

#include "pluginmanager.h"

namespace Ui {
class PluginManagerDialog;
}

class PluginManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginManagerDialog(QWidget *parent = 0);
    ~PluginManagerDialog();

    void setPluginManager(PluginManager *pl);

    void accept();
private:
    Ui::PluginManagerDialog *ui;

    PluginManager *pluginManager;
};

#endif // PLUGINMANAGERDIALOG_H
