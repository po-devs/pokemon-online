#ifndef QSCROLLDOWNTEXTBROWSER_H
#define QSCROLLDOWNTEXTBROWSER_H

#include <QTextBrowser>
#include <QMenu>

class QScrollDownTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    QScrollDownTextBrowser(QWidget *parent=0);

    void setAutoClear(bool a) {
        autoClear = a;
    }

    void keepLines(int numberOfLines);

public slots:
    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);
    void clear(); // Overriden to make linecount zero.

private:
    int linecount;
    bool autoClear;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void resizeEvent(QResizeEvent *e);

    void updateBarStatus();

    QMenu *menu;
private:
    bool scrolledToMax;
};

#endif // QSCROLLDOWNTEXTBROWSER_H
