#ifndef OTHERWIDGETS_H
#define OTHERWIDGETS_H

#include <QtGui>

class QCompactTable : public QTableWidget
{
    Q_OBJECT
protected:
    int sizeHintForRow ( int row ) const;
public:
    QCompactTable(int row, int column);
};

/* a widget that allows giving a title to another widget */
class QEntitled : public QWidget
{
    Q_OBJECT
private:
    QLabel *m_title;
    QWidget *m_widget;
    QVBoxLayout *m_layout;

public:
    QEntitled(const QString &title = "Title", QWidget *widget = 0);
    void setTitle(const QString &title);
    void setWidget(QWidget *widget);
};

class QCenteredWidget : public QWidget
{
    Q_OBJECT
 public:
    QCenteredWidget(QWidget* parent=0);
};

#endif // OTHERWIDGETS_H
