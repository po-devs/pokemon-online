#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_addFiles_clicked();
    void on_clearFiles_clicked();
    void on_done_clicked();
private:
    Ui::MainWindow *ui;

    QStringList files;
};

#endif // MAINWINDOW_H
