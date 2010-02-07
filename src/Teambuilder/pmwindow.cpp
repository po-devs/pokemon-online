#include "pmwindow.h"
#include "../Utilities/otherwidgets.h"

PMWindow::PMWindow(int id, const QString &ownName, const QString &name, const QString &content)
    : m_ownName(ownName)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    this->id() = id;
    changeName(name);

    QGridLayout *l = new QGridLayout(this);
    this->setLayout(l);

    m_mainwindow = new QScrollDownTextEdit();
    m_textToSend = new QLineEdit();

    l->addWidget(m_mainwindow, 0,0,1,2);
    l->addWidget(m_textToSend, 1,0,1,2);

    m_challenge = new QPushButton("&Challenge");
    m_send = new QPushButton("&Send");

    l->addWidget(m_challenge,2,0);
    l->addWidget(m_send,2,1);

    printLine(content, false);

    connect(m_textToSend, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(m_send, SIGNAL(clicked()), this, SLOT(sendMessage()));

    QSignalMapper *s = new QSignalMapper(this);
    s->setMapping(m_challenge, id);
    connect(m_challenge, SIGNAL(clicked()), s, SLOT(map()));
    connect(s, SIGNAL(mapped(int)), SIGNAL(challengeSent(int)));
}

void PMWindow::changeName(const QString &newname)
{
    this->m_name = newname;
    setWindowTitle(newname);
}

void PMWindow::printLine(const QString &line, bool self)
{
    if (line.trimmed().length() == 0)
        return;
    if (self) {
        printHtml(toBoldColor(escapeHtml(m_ownName) + ": ", Qt::darkBlue) + escapeHtml(line));
    } else {
        printHtml(toBoldColor(escapeHtml(name()) + ": ", Qt::darkGray) + escapeHtml(line));
    }
}

void PMWindow::printHtml(const QString &htmlCode)
{
    m_mainwindow->insertHtml(htmlCode + "<br />");
}

void PMWindow::sendMessage()
{
    QString str = m_textToSend->text().trimmed();
    m_textToSend->clear();

    if (str.length() == 0) {
        return;
    }

    emit messageEntered(id(), str);
    printLine(str, true);
}
