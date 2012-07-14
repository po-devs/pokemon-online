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
    ui->battlescripts->insertPlainText(ScriptUtils::loadScripts(ScriptUtils::BattleScripts));
    loadSettings(this, QSize(583, 446));

    QSettings s;
    s.beginGroup("ScriptWindow");
    ui->checkBox->setChecked(s.value("safeScripts", true).toBool());
    ui->warn->setChecked(s.value("warn", false).toBool());
    s.endGroup();

    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(safeScriptsChanged(int)));
    connect(ui->warn, SIGNAL(stateChanged(int)), this, SLOT(warningsChanged(int)));
}

void ScriptWindow::accept()
{
    QDir d(appDataPath("Scripts/", true));
    QFile f(d.absoluteFilePath("scripts.js"));
    f.open(QIODevice::WriteOnly);
    QString text = ui->scripts->toPlainText();
    f.write(text.toUtf8());

    QFile f2(d.absoluteFilePath("battlescripts.js"));
    f2.open(QIODevice::WriteOnly);
    QString text2 = ui->battlescripts->toPlainText();
    f2.write(text2.toUtf8());

    emit scriptChanged(text);
    emit battleScriptChanged(text2);

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

void ScriptWindow::warningsChanged(int newStatus)
{
    QSettings s;

    s.beginGroup("ScriptWindow");

    if (newStatus == Qt::Checked) {
        s.setValue("warn", true);
        emit warningsChanged(true);
    } else {
        s.setValue("warn", false);
        emit warningsChanged(false);
    }

    s.endGroup();
}
