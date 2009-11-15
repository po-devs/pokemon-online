#include "otherwidgets.h"
#include <QtGui>

QCompactTable::QCompactTable(int row, int column)
        : QTableWidget(row, column)
{
}

int QCompactTable::sizeHintForRow(int) const
{
    return 0;
}


QEntitled::QEntitled(const QString &title, QWidget *widget)
{
    m_layout = new QVBoxLayout(this);

    /* The space is there for correct alignment of the title */
    m_title = new QLabel( title);
    m_title->setMaximumHeight(17);

    m_layout->addWidget(m_title, 0, Qt::AlignBottom);
    if (widget)
        m_widget = widget;
    else
        m_widget = new QWidget();
    m_layout->addWidget(m_widget, 0, Qt::AlignTop);
    m_title->setBuddy(m_widget);

    /* Makes the title/items stick together */
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
}

void QEntitled::setWidget(QWidget *widget)
{
    m_layout->removeWidget(m_widget);
    m_widget = widget;
    m_layout->addWidget(m_widget, 0, Qt::AlignTop);
    m_title->setBuddy(m_widget);
}

void QEntitled::setTitle(const QString &title)
{
    m_title->setText(title);
}

QImageButton::QImageButton(const QString &normal, const QString &hovered)
            : myPic(normal), myHoveredPic(hovered)
{
    setFixedSize(myPic.size());
}

QSize QImageButton::sizeHint() const
{
    return myPic.size();
}

QSize QImageButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize QImageButton::maximumSize() const
{
    return sizeHint();
}

void QImageButton::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    if (!underMouse())
        painter.drawPixmap(e->rect(), myPic, e->rect());
    else
        painter.drawPixmap(e->rect(), myHoveredPic, e->rect());
}

QImageBackground::QImageBackground(const QString &imagePath)
        : myBackground(imagePath)
{
    if (!myBackground.isNull())
        setFixedSize(myBackground.size());
}

QSize QImageBackground::sizeHint() const
{
    return myBackground.size();
}

QSize QImageBackground::minimumSizeHint() const
{
    return sizeHint();
}

QSize QImageBackground::maximumSize() const
{
    return sizeHint();
}

void QImageBackground::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.drawPixmap(e->rect(), myBackground, e->rect());
}

QIdListWidgetItem::QIdListWidgetItem(int id, const QString &text)
	: QListWidgetItem(text), myid(id)
{
}

int QIdListWidgetItem::id() const
{
    return myid;
}
