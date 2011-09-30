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

            if (res != "pokemon" && res != "item") {
                goto end;
            }

            {
                QVariant ret;

                if (res == "pokemon") {
                    QStringList params = url.section(':', 1).split('&');

                    int gen = 5;
                    int gender = 0;
                    Pokemon::uniqueId num = Pokemon::NoPoke;
                    bool shiny=false;
                    bool back = false;

                    foreach (QString param, params) {
                        QString par = param.section('=', 0,0);
                        QString val = param.section('=', 1);

                        if (par.length() > 0 && par[0].isDigit() && val.length() == 0) {
                            val = par;
                            par = "num";
                        }

                        if (par == "gen") {
                            gen = val.toInt();
                        } else if (par == "num") {
                            num = Pokemon::uniqueId(val.section('-', 0,0).toInt(), val.section('-', 1).toInt());
                        } else if (par == "shiny") {
                            shiny = val == "true";
                        } else if (par == "gender") {
                            gender = val == "male" ? Pokemon::Male : (val == "female"?Pokemon::Female : Pokemon::Neutral);
                        } else if (par == "back") {
                            back = val == "true";
                        }
                    }

                    ret = PokemonInfo::Picture(num, gen, gender, shiny, back);
                } else if (res == "item") {
                    int item = url.section(':', 1).toInt();
                    ret = ItemInfo::Icon(item);
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
