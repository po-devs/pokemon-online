#ifndef MODSWINDOW_H
#define MODSWINDOW_H

#include <QWidget>

namespace Ui {
class ModsWindow;
}

class ModsWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ModsWindow(QWidget *parent = 0);
    ~ModsWindow();
signals:
    void modChanged(const QString&);
public slots:
    void accepted();
    void exportMod();
private slots:
    void fileNameReceived(const QString &path);
private:
    Ui::ModsWindow *ui;
};

#endif // MODSWINDOW_H
