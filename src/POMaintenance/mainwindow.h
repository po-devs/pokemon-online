#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setSuccessIndicator(bool*);
    void run(const QString &source);
public slots:
    void progress(const QString &progress);
    void problem(const QString &file);
    void finished();
private:
    Ui::MainWindow *ui;

    bool *success;

    Core c;
};

#endif // MAINWINDOW_H
