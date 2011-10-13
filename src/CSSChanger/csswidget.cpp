#include "csswidget.h"
#include "../Teambuilder/theme.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore>

CssWidget::CssWidget() {
    setupUi(this);

    QFile stylesheet(Theme::path("default.css"));
    stylesheet.open(QIODevice::ReadOnly);
    QString sheet = QString::fromUtf8(stylesheet.readAll());

    data.stylesheet = sheet;

    QRegExp regExp("(#[A-Fa-f0-9]{6})\\b");

    int pos = 0;

    while ((pos = regExp.indexIn(sheet, pos)) != -1) {
        data.colors << Data::PosValue(pos, QColor(regExp.cap(1)));
        pos += regExp.matchedLength();
    }
}
