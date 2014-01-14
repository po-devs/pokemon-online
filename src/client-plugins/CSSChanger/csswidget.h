#ifndef CSSWIDGET_H
#define CSSWIDGET_H

#include "ui_dialog.h"
#include "data.h"

struct ThemeAccessor;

class CssWidget : public QDialog, public Ui::Dialog {
Q_OBJECT
public:
    CssWidget(ThemeAccessor* theme);

    Data data;

    void setupList();
    void setupGrid();
public slots:
    void onColorChanged(int num, QColor color);
    void onApply();
    void onAccept();
    void updateGrid();
private slots:
    void on_buttonBox_clicked(QAbstractButton*);
    void on_tabWidget_currentChanged(QWidget*);
    void openMassColor();
private:
    ThemeAccessor *theme;
    QString path;
};

#endif // CSSWIDGET_H
