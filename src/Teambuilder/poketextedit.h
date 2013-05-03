#ifndef POKETEXTEDIT_H
#define POKETEXTEDIT_H

#include "../Utilities/qscrolldowntextbrowser.h"

class PokeTextEdit : public QScrollDownTextBrowser
{
public:
    PokeTextEdit(QWidget *parent=0) : QScrollDownTextBrowser(parent){}
    QVariant loadResource(int type, const QUrl &name);
};

class SmallPokeTextEdit : public PokeTextEdit
{
public:
    SmallPokeTextEdit();
    QSize sizeHint() const;

    void setText(const QString &text);
protected:
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *e);

    void adaptSize();
};

#endif // POKETEXTEDIT_H
