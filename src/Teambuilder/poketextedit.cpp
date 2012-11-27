#include "poketextedit.h"
#include "../PokemonInfo/pokemoninfo.h"

QVariant PokeTextEdit::loadResource(int type, const QUrl &name)
{
    if (type != QTextDocument::ImageResource) {
        goto end;
    }

    {
        QString url = name.toString();

        if (url.indexOf(":") <= 0) {
            goto end;
        }

        {
            QString res = url.section(':', 0, 0);

            if (res != "pokemon" && res != "item" && res != "icon") {
                goto end;
            }

            {
                QVariant ret;

                QString info = url.section(":", 1);
                if (res == "pokemon") {
                    ret = PokemonInfo::Picture(info);
                } else if (res == "item") {
                    int item = info.toInt();
                    ret = ItemInfo::Icon(item);
                } else if (res == "icon") {
                    Pokemon::uniqueId num = info.toInt();
                    ret = PokemonInfo::Icon(num);
                }

                return ret;
            }
        }
    }

end:
    return QScrollDownTextBrowser::loadResource(type, name);
}

SmallPokeTextEdit::SmallPokeTextEdit()
{
    setMinimumSize(QSize(0, 0));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SmallPokeTextEdit::adaptSize()
{
    int height = document()->size().toSize().height();

    if (height > 0) {
        height = height + 4;
    }
    setFixedHeight(height);
}

QSize SmallPokeTextEdit::sizeHint() const
{
    return document()->size().toSize();
}

void SmallPokeTextEdit::setText(const QString &text)
{
    PokeTextEdit::setText(text);

    adaptSize();
}

void SmallPokeTextEdit::resizeEvent(QResizeEvent *e)
{
    PokeTextEdit::resizeEvent(e);
    adaptSize();
}

void SmallPokeTextEdit::showEvent(QShowEvent *e)
{
    PokeTextEdit::showEvent(e);
    adaptSize();
}
