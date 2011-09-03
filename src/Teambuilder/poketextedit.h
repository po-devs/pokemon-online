#ifndef POKETEXTEDIT_H
#define POKETEXTEDIT_H

#include "../Utilities/otherwidgets.h"

class PokeTextEdit : public QScrollDownTextBrowser
{
public:
    QVariant loadResource(int type, const QUrl &name);
};

#endif // POKETEXTEDIT_H
