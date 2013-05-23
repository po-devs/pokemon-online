#include "modswindow.h"
#include "ui_modswindow.h"
#include "../PokemonInfo/pokemoninfo.h"

ModsWindow::ModsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModsWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(accepted()));
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(close()));
    connect(ui->buttonBox, SIGNAL(rejected()), SLOT(close()));

    ui->comboBox->addItem("No Mods");

    QSettings s("config", QSettings::IniFormat);
    QString currentMod = s.value("Mods/CurrentMod").toString();

    foreach(QString mod, PokemonInfoConfig::availableMods()) {
        ui->comboBox->addItem(mod);

        if (mod == currentMod) {
            ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
        }
    }

    show();
}

void ModsWindow::accepted()
{
    QSettings s("config", QSettings::IniFormat);

    QString mod = ui->comboBox->currentIndex() == 0 ? QString() : ui->comboBox->currentText();

    if (mod != s.value("Mods/CurrentMod")) {
        s.setValue("Mods/CurrentMod", mod);
        emit modChanged(mod);
    }
}

ModsWindow::~ModsWindow()
{
    delete ui;
}
