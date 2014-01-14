#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), success(NULL)
{
    ui->setupUi(this);

    connect(&c, SIGNAL(progress(QString)), SLOT(progress(QString)));
    connect(&c, SIGNAL(problem(QString)), SLOT(problem(QString)));
    connect(&c, SIGNAL(finished()), SLOT(finished()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSuccessIndicator(bool *b)
{
    success = b;
}

void MainWindow::run(const QString &source)
{
    c.setSource(source);
    c.start();
}

void MainWindow::finished()
{
    close();
}

void MainWindow::progress(const QString &progress)
{
    ui->label->setText(progress);
    ui->plainTextEdit->insertPlainText(progress + "\n");
}

void MainWindow::problem(const QString &file)
{
    int decision = QMessageBox::question(this, tr("Impossible to update file"), tr("File %1 couldn't be updated, what do you want to do?").arg(file),
                          QMessageBox::Abort, QMessageBox::Ignore, QMessageBox::Retry);

    if (decision == QMessageBox::Abort) {
        if (success) {
            *success = false;
        }
        c.stop();
    } else if (decision == QMessageBox::Ignore) {
        c.skipFile();
        c.restart();
    } else { /* QMessageBox::Retry */
        c.restart();
    }
}
