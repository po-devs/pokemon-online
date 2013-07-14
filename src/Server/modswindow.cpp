#include <QFileDialog>

#include "../Utilities/ziputils.h"
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
    connect(ui->export_2, SIGNAL(clicked()), SLOT(exportMod()));

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

void ModsWindow::exportMod()
{
    QString mod = ui->comboBox->currentIndex() == 0 ? QString() : ui->comboBox->currentText();

    if (mod.length() <= 0) {
        return;
    }

    QString defaultPath = PokemonInfoConfig::dataRepo();
    QFileDialog *f = new QFileDialog(NULL, QObject::tr("Exporting the mod"),defaultPath, QObject::tr("Zip (*.zip)"));
    //f->setWindowFlags(Qt::Window); //maybe the reason for crashes for some people
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->setAcceptMode(QFileDialog::AcceptSave);

    f->show();
    f->setProperty("mod", mod);

    QObject::connect(f, SIGNAL(fileSelected(QString)), this, SLOT(fileNameReceived(QString)));
}

static void recurseZip(Zip &zip, const QDir& d, const QString &path);

void ModsWindow::fileNameReceived(const QString &path)
{
    QString mod = sender()->property("mod").toString();

    Zip zip;
    zip.create(path);

    QDir d(PokemonInfoConfig::dataRepo() + "Mods");

    d.cd(mod);

    recurseZip(zip, d, "");
}

static void recurseZip(Zip &zip, const QDir& d, const QString &path) {
    QStringList files = d.entryList(QDir::Files | QDir::Readable);

    foreach(const QString &file, files) {
        zip.addFile(d.absoluteFilePath(file), path + QFileInfo(file).fileName());
    }

    QStringList dirs = d.entryList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    foreach(const QString &file, dirs) {
        QDir d2 = d;
        d2.cd(file);
        recurseZip(zip, d2, path + d2.dirName() + "/");
    }
}

ModsWindow::~ModsWindow()
{
    delete ui;
}
