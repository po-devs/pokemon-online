#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include <stdexcept>

#include "pluginmanagerwidget.h"
#include "pluginmanager.h"

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
    fd->setDirectory(QDir(pl.directory()).absolutePath());
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

    try {
        pl.addPlugin(rel);
    } catch (const std::runtime_error & ex) {
        emit error(ex.what());
        return;
    }

    list->clear();
    list->addItems(pl.getPlugins());

    emit pluginListChanged();
}
