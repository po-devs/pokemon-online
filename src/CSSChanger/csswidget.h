#ifndef CSSWIDGET_H
#define CSSWIDGET_H

#include "ui_dialog.h"

class CssWidget : public QDialog, public Ui::Dialog {
public:
    CssWidget();

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
};

#endif // CSSWIDGET_H
