#ifndef QVERTICALSCROLLAREA_H
#define QVERTICALSCROLLAREA_H

#include <QScrollArea>

class QVerticalScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit QVerticalScrollArea(QWidget *parent = 0);
    
    void setWidget(QWidget *w);
protected:
    bool eventFilter(QObject *, QEvent *);
signals:
    
public slots:
    
};

#endif // QVERTICALSCROLLAREA_H
