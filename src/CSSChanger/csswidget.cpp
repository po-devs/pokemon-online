#include "csswidget.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore>
#include "../Teambuilder/themeaccessor.h"
#include "colorchoicewidget.h"
#include <QDialogButtonBox>

CssWidget::CssWidget(ThemeAccessor* theme) : theme(theme) {
    setupUi(this);

    path = theme->path("default.css");
    QFile stylesheet(path);
    stylesheet.open(QIODevice::ReadOnly);
    QString sheet = QString::fromUtf8(stylesheet.readAll());

    data.stylesheet = sheet;

    QRegExp regExp("(#[A-Fa-f0-9]{6})\\b");
    QRegExp comment("/\\*([^*]|\\*(?!/))*\\*/");

    int pos = 0;

    while ((pos = regExp.indexIn(sheet, pos)) != -1) {

        int acc = sheet.lastIndexOf("{", pos);
        if (acc != -1) {
            if (acc > 0) {
                acc -= 1;
            }
            int acc2 = sheet.lastIndexOf("}", acc);

            if (acc2 == -1) {
                acc2 = 0;
            } else {
                acc2++;
            }
            QString desc = sheet.mid(acc2, acc-acc2).replace(comment, "").trimmed();

            data.colors << Data::PosValue(pos, QColor(regExp.cap(1)), desc);
        } else {
            data.colors << Data::PosValue(pos, QColor(regExp.cap(1)));
        }

        pos += regExp.matchedLength();
    }

    setupList();
}

void CssWidget::setupList()
{
    for (int i = 0; i < data.colors.size(); i++) {
        ColorChoiceWidget *w = new ColorChoiceWidget();
        w->setNumber(i);
        w->setColor(data.colors[i].value);
        if (!data.colors[i].desc.isEmpty()) {
            w->setDesc(data.colors[i].desc);
        }
        colorsList->addWidget(w);

        connect(w, SIGNAL(colorChanged(int,QColor)), SLOT(onColorChanged(int,QColor)));
    }
}

void CssWidget::onColorChanged(int num, QColor color)
{
    data.colors[num].value = color;
    data.stylesheet.replace(data.colors[num].pos, color.name().length(), color.name());
}

void CssWidget::on_buttonBox_clicked(QAbstractButton *b)
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(b);

    if (role == QDialogButtonBox::RejectRole) {
        reject();
    } else if (role == QDialogButtonBox::ApplyRole) {
        qApp->setStyleSheet(data.stylesheet);
    } else if (role == QDialogButtonBox::AcceptRole) {
        qApp->setStyleSheet(data.stylesheet);
        QFile out(path);
        out.open(QIODevice::WriteOnly);
        out.write(data.stylesheet.toUtf8());
        out.close();;
        accept();
    }
}
