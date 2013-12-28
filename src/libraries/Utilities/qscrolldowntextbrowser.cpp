#include <QScrollBar>
#include <QContextMenuEvent>

#include "qscrolldowntextbrowser.h"

QScrollDownTextBrowser::QScrollDownTextBrowser(QWidget *parent) : QTextBrowser(parent)
{
    autoClear = true;
    scrolledToMax = false;
    setReadOnly(true);
    setOpenExternalLinks(true);
    linecount = 0;
    // Take standard menu, add clear to it, save for later use.
    menu = NULL;
}

void QScrollDownTextBrowser::insertHtml(const QString &text)
{
    QScrollBar * b = verticalScrollBar();
    if (linecount >= 2000 && autoClear) {
        keepLines(1000);
        moveCursor(QTextCursor::End);
        linecount = 1000;
        QTextBrowser::insertHtml(text);
        b->setValue(b->maximum());
        return;
    }

    int f = b->value();
    int e = b->maximum();

    /* Stores cursor state before moving it in case we need it later */
    QTextCursor cursor = this->textCursor();

    moveCursor(QTextCursor::End);
    QTextBrowser::insertHtml(text);

    /* If we had something highlighted, restore it */
    if (cursor.selectionEnd() != cursor.selectionStart()) {
        setTextCursor(cursor);
    }

    if(f != e)
    {
        scrolledToMax = false;
        b->setValue(f);
    }
    else
    {
        scrolledToMax = true;
        b->setValue(b->maximum());
    }
    linecount++;
}

void QScrollDownTextBrowser::keepLines(int numberOfLines)
{
    setReadOnly(false);
    moveCursor(QTextCursor::Start);
    moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);

    while (numberOfLines > 0) {
        --numberOfLines;
        moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
    }

    textCursor().removeSelectedText();
    setReadOnly(true);
}

void QScrollDownTextBrowser::insertPlainText(const QString &text)
{
    QScrollBar * b = verticalScrollBar();
    if (linecount >= 2000 && autoClear) {
        keepLines(1000);
        moveCursor(QTextCursor::End);
        linecount = 1000;
        QTextBrowser::insertPlainText(text);
        b->setValue(b->maximum());
        return;
    }

    int f = b->value();
    int e = b->maximum();

    moveCursor(QTextCursor::End);
    QTextBrowser::insertPlainText(text);

    if(b->value() != e)
    {
        scrolledToMax = false;
        b->setValue(f);
    }
    else
    {
        scrolledToMax = true;
        b->setValue(b->maximum());
        moveCursor(QTextCursor::End);
    }
    linecount++;
}

void QScrollDownTextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    if (menu) {
        menu->deleteLater();
    }
    menu = createStandardContextMenu(event->pos());
    menu->setParent(this);
    QAction *action = menu->addAction(tr("Clear"));
    connect(action, SIGNAL(triggered()), this, SLOT(clear()));
    menu->exec(event->globalPos());
}

void QScrollDownTextBrowser::clear()
{
    QTextBrowser::clear(); // Call parent.
    linecount = 0;
}

void QScrollDownTextBrowser::mouseMoveEvent(QMouseEvent *ev)
{
    QTextBrowser::mouseMoveEvent(ev);
    updateBarStatus();
}

void QScrollDownTextBrowser::mousePressEvent(QMouseEvent *ev)
{
    QTextBrowser::mousePressEvent(ev);
    updateBarStatus();
}

void QScrollDownTextBrowser::resizeEvent(QResizeEvent *e)
{
    QTextBrowser::resizeEvent(e);

    if (scrolledToMax) {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
}

void QScrollDownTextBrowser::updateBarStatus()
{
    scrolledToMax = verticalScrollBar()->maximum() == verticalScrollBar()->value();
}
