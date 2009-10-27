#include "otherwidgets.h"

QCompactTable::QCompactTable(int row, int column)
        : QTableWidget(row, column)
{
}

int QCompactTable::sizeHintForRow(int row) const
{
    (void) row;
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

QCenteredWidget::QCenteredWidget(QWidget *parent)
        : QWidget(parent)
{
    QDesktopWidget desktop;
    QRect deskrect = desktop.screenGeometry();
    int x = deskrect.width()/2 - width()/2;
    int y = deskrect.height()/2 - height()/2;
    move(x,y);
};
