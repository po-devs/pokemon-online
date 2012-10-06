#ifndef SMOGONPLUGIN_H
#define SMOGONPLUGIN_H

#include <QWidget>

class SmogonPlugin : public QWidget
{
    Q_OBJECT
    
public:
    SmogonPlugin(QWidget *parent = 0);
    ~SmogonPlugin();
};

#endif // SMOGONPLUGIN_H
