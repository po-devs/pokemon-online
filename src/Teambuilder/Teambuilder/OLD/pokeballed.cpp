#include <QLabel>
#include <QHBoxLayout>
#include "pokeballed.h"
#include "theme.h"

/**********************************/
/**** POKEBALLED ******************/
/**********************************/

Pokeballed::Pokeballed(QWidget *w) {
    init(w);
}

Pokeballed::Pokeballed() {

}

void Pokeballed::init(QWidget *w)
{
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setMargin(0);

    QLabel *icon = new QLabel();
    icon->setPixmap(Theme::BlueBall());
    h->addWidget(icon);

    h->addWidget(w,100,Qt::AlignLeft);
}

/////////////////////////////////////
//// TITLED WIDGET //////////////////
/////////////////////////////////////

TitledWidget::TitledWidget(const QString &title, QWidget *w)
{
    QVBoxLayout *v = new QVBoxLayout(this);
    v->setMargin(0);

    QLabel *l = new QLabel(title);
    l->setObjectName("NormalText");

    v->addWidget(new Pokeballed(l));
    v->addWidget(w,100,Qt::AlignTop);
    l->setBuddy(w);
}

