#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <QDialog>
#include "designerplugin.h"

namespace Ui {
class DesignerWidget;
}

class DesignerWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DesignerWidget(DesignerPlugin *plugin);
    ~DesignerWidget();
    
    void accept();
    int limitForTab();
public slots:
    void liveReloadChanged(bool checked);
    void infoTextChanged();
    void reloadPressed();
private:
    Ui::DesignerWidget *ui;
    DesignerPlugin *plugin;
    void updateUi();

    bool liveReload;
};

#endif // DESIGNERWIDGET_H
