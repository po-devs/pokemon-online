#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include "../Utilities/functions.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QString appData = appDataPath("SettingsPlugin", true);

    QSettings s;
    QSettings out(appData + "/settings.ini", QSettings::IniFormat);

    foreach(QString k, s.allKeys()) {
        out.setValue(k,s.value(k));
    }

    out.sync();

    QFile in(appData+"/settings.ini");
    in.open(QIODevice::ReadOnly);
    ui->textZone->insertPlainText(QString::fromUtf8(in.readAll()));
}

void SettingsDialog::accept()
{
    QString appData = appDataPath("SettingsPlugin",true);
    QFile out(appData + "/settings.ini");
    out.open(QIODevice::WriteOnly);

    out.write(ui->textZone->toPlainText().toUtf8());
    out.close();

    QSettings s;
    QSettings in(appData + "/settings.ini", QSettings::IniFormat);

    s.clear();
    foreach(QString k, in.allKeys()) {
        s.setValue(k,in.value(k));
    }
    s.sync();

    QDialog::accept();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
