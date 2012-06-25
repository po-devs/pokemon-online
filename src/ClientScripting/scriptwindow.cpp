#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "../Utilities/functions.h"

ScriptWindow::ScriptWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScriptWindow)
{
    ui->setupUi(this);

    QDir d(appDataPath("Scripts/", true));
    QFile f(d.absoluteFilePath("scripts.js"));
    f.open(QIODevice::ReadOnly);

    ui->scripts->insertPlainText(QString::fromUtf8(f.readAll()));
}

void ScriptWindow::accept() {
    QDir d(appDataPath("Scripts/", true));
    QFile f(d.absoluteFilePath("scripts.js"));
    f.open(QIODevice::WriteOnly);

    QString text = ui->scripts->toPlainText();

    f.write(text.toUtf8());

    emit scriptChanged(text);

    QDialog::accept();
}

ScriptWindow::~ScriptWindow()
{
    delete ui;
}
