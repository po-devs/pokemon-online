#include <QFileDialog>
#include <QStringListModel>
#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Shared/config.h"
#include "../Utilities/ziputils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringListModel *m = new QStringListModel();
    m->setStringList(QStringList() << "version.ini");
    files.push_back("version.ini");

    ui->filesList->setModel(m);

    QSettings test("version.ini", QSettings::IniFormat);

    ui->version->setPlaceholderText(test.value("version").toString());
    ui->updateId->setText(test.value("updateId").toString());
}

void MainWindow::on_addFiles_clicked()
{
    QStringList lfiles = QFileDialog::getOpenFileNames();

    QDir d;

    foreach(QString f, lfiles) {
        files.append(d.relativeFilePath(f));
    }

    files = files.toSet().toList(); // Clears duplicates
    qSort(files);

    if (ui->filesList->model()) {
        ui->filesList->model()->deleteLater();
    }
    ui->filesList->setModel(new QStringListModel(files));
}

void MainWindow::on_clearFiles_clicked()
{
    files.clear();
    files.push_back("version.ini");
    if (ui->filesList->model()) {
        ui->filesList->model()->deleteLater();
    }
    ui->filesList->setModel(new QStringListModel(files));
}

void MainWindow::on_done_clicked()
{
    QString version = (ui->version->text().isEmpty() ? VERSION : ui->version->text());
    QString os = (ui->os->text().isEmpty() ? "all" : ui->os->text());
    QString updateId = ui->updateId->text();

    QString zipFile = "update-" + version + "-" + os + "-" + updateId + ".zip";

    Zip zip;
    zip.create(zipFile);

    foreach(QString file, files) {
        zip.addFile(file);
    }

    //zip.addMemoryFile(QString("[General]\nversion=%1\nupdateId=%2\nos=%3\n").arg(version, updateId, os).toUtf8(), "update.dat");
    zip.addMemoryFile(QString(ui->changeLog->toPlainText()).toUtf8(), "changelog.txt");
    zip.writeArchive();

    statusBar()->showMessage(QString("File %1 created!").arg(zipFile));
}

MainWindow::~MainWindow()
{
    delete ui;
}
