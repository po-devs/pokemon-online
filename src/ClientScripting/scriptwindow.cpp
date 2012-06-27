#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "../Utilities/functions.h"
#include "scriptutils.h"

ScriptWindow::ScriptWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScriptWindow)
{
    ui->setupUi(this);

    ui->scripts->insertPlainText(ScriptUtils::loadScripts());
    loadSettings(this, QSize(583, 446));

    QSettings s;
    s.beginGroup("ScriptWindow");
    if (s.childKeys().contains("safeScripts")) {
        ui->checkBox->setChecked(s.value("safeScripts").toBool());
    }
    s.endGroup();

    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(safeScriptsChanged(int)));
}

void ScriptWindow::accept()
{
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
    writeSettings(this);
    delete ui;
}

void ScriptWindow::safeScriptsChanged(int newStatus)
{
    QSettings s;

    s.beginGroup("ScriptWindow");

    if (newStatus == Qt::Checked) {
        s.setValue("safeScripts", true);
        emit safeScriptsChanged(true);
    } else {
        s.setValue("safeScripts", false);
        emit safeScriptsChanged(false);
    }

    s.endGroup();
}
