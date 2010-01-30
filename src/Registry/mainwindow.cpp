#include "mainwindow.h"
#include "registry.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Registry for Pokemon Online"));

    setCentralWidget(myserver = new Registry());
    resize(500,500);
}

MainWindow::~MainWindow()
{
}
