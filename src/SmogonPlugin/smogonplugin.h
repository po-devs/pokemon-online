#ifndef SMOGONPLUGIN_H
#define SMOGONPLUGIN_H

#include <QMainWindow>

namespace Ui {
class SmogonPlugin;
}

class SmogonPlugin : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SmogonPlugin(QWidget *parent = 0);
    ~SmogonPlugin();
    
private:
    Ui::SmogonPlugin *ui;
};

#endif // SMOGONPLUGIN_H
