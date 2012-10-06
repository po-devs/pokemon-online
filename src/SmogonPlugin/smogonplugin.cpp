#include "smogonplugin.h"
#include "ui_smogonplugin.h"

SmogonPlugin::SmogonPlugin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SmogonPlugin)
{
    ui->setupUi(this);
}

SmogonPlugin::~SmogonPlugin()
{
    delete ui;
}
