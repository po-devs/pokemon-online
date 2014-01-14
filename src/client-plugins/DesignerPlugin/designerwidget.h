#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include "designerplugin.h"
#include "ui_designerwidget.h"

static const int limitPerTab[] {
    300,
    500,
    80000
};

class DesignerWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DesignerWidget(DesignerPlugin *plugin);
    ~DesignerWidget();

    int limitForTab;

    inline QPlainTextEdit* tabInput()
    {
        switch (tab) {
        case 1: // server description
            return ui->descInput;
        case 2: // server announcement
            return ui->annInput;
        default: // 0 - trainer info
            return ui->infoInput;
        }
    }

    inline QScrollDownTextBrowser* tabOutput()
    {
        switch (tab) {
        case 1: // server description
            return ui->descOutput;
        case 2: // server announcement
            return ui->annOutput;
        default: // 0 - trainer info
            return ui->infoOutput;
        }
    }
public slots:
    void liveReloadChanged(bool checked);
    void textChanged();
    void tabChanged(int id);
    void reloadPressed();
private:
    Ui::DesignerWidget* ui;
    DesignerPlugin* plugin;
    void updateUi();

    int tab;

    bool liveReload;
};

#endif // DESIGNERWIDGET_H
