#ifndef CSSWIDGET_H
#define CSSWIDGET_H

#include "ui_dialog.h"

class ThemeAccessor;

class CssWidget : public QDialog, public Ui::Dialog {
Q_OBJECT
public:
    CssWidget(ThemeAccessor* theme);

    struct Data {
        struct PosValue {
            int pos;
            QColor value;

            PosValue(int pos=0, QColor color = QColor()) : pos(pos), value(color) {}
        };

        QString stylesheet;
        QList<PosValue> colors;
    };
    Data data;

    void setupList();
public slots:
    void onColorChanged(int num, QColor color);
private slots:
    void on_buttonBox_clicked(QAbstractButton*);
private:
    ThemeAccessor *theme;
    QString path;
};

#endif // CSSWIDGET_H
