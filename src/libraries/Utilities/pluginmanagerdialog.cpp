#include <stdexcept>
#include <QMessageBox>

#include "pluginmanagerdialog.h"
#include "ui_pluginmanagerdialog.h"

PluginManagerDialog::PluginManagerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginManagerDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
}

PluginManagerDialog::~PluginManagerDialog()
{
    delete ui;
}

void PluginManagerDialog::setPluginManager(PluginManager *pl)
{
    pluginManager = pl;

    QMap<QString, QString> available = pl->availablePlugins();

    QStringList loadedPlugins = pl->getPlugins();

    foreach(QString k, available.keys())
    {
        QString pl = available[k];

        ui->plugins->addItem(pl);

        QListWidgetItem *item = ui->plugins->item(ui->plugins->count()-1);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, k);

        if (loadedPlugins.contains(pl)) {
            ui->plugins->item(ui->plugins->count()-1)->setCheckState(Qt::Checked);
        }
    }

    ui->plugins->sortItems();
}

void PluginManagerDialog::accept()
{
    QStringList loadedPlugins = pluginManager->getPlugins();

    for (int i = 0; i < ui->plugins->count(); i++) {
        QString pl = ui->plugins->item(i)->text();

        if (ui->plugins->item(i)->checkState() == Qt::Checked) {
            if (loadedPlugins.contains(pl)) {
                continue;
            } else {
                try {
                    pluginManager->addPlugin(ui->plugins->item(i)->data(Qt::UserRole).toString());
                } catch (const std::runtime_error & ex) {
                    QMessageBox::warning(topLevelWidget(), tr("Error loading plugin"), ex.what());
                    return;
                }
            }
        } else {
            if (!loadedPlugins.contains(pl)) {
                continue;
            } else {
                pluginManager->freePlugin(pl);
            }
        }
    }

    QDialog::accept();
}
