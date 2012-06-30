#include "pmsystem.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/functions.h"

// TO-DO: Begin able to drag a tab outside of it's tab widget to separate the PM. I didn't do it because i
// think it would break part of the new system.

PMSystem::PMSystem(bool withTabs)
{
    QVBoxLayout *myLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Private Messages"));
    setMinimumSize(300, 300);
    myPMs = new QExposedTabWidget();
    myPMs->setTabsClosable(true);
    myPMs->setMovable(true);
    myLayout->addWidget(myPMs);
    setLayout(myLayout);

    tabbedPMs = withTabs;

    connect(myPMs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(myPMs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

PMSystem::~PMSystem()
{
}

void PMSystem::startPM(PMStruct *newPM)
{
    if(tabbedPMs) {
        newPM->setWindowFlags(Qt::Widget);
        myPMs->addTab(newPM, newPM->name());
        checkTabbing();
    } else {
        newPM->setWindowFlags(Qt::Window);
        newPM->show();
    }
    myPMWindows.insert(newPM->id(), newPM);

    connect(newPM, SIGNAL(messageReceived(PMStruct*)), this, SLOT(messageReceived(PMStruct*)));
    connect(newPM, SIGNAL(idChanged(int,int)), SLOT(changeId(int,int)));
    connect(newPM, SIGNAL(destroyed(int,QString)), SLOT(removePM(int)));
}

void PMSystem::removePM(int pmid)
{
    PMStruct *pm = myPMWindows.take(pmid);

    if (pm) {
        pm->deleteLater();

        if (tabbedPMs) {
            for (int i = 0; i < myPMs->count(); i++) {
                if (myPMs->widget(i) == pm) {
                    myPMs->removeTab(i);
                    break;
                }
            }
            checkTabbing();
        }
    }
}

void PMSystem::closeTab(int tabNum)
{
    PMStruct *widget = (PMStruct*) myPMs->widget(tabNum);
    removePM(widget->id());
}

void PMSystem::checkTabbing()
{
    if(myPMs->count() <= 0) {
        hide();
    } else {
        show();
    }
}

void PMSystem::tabChanged(int tabNum)
{
    if(myPMs->count() <= 0) {
        return;
    }
    PMStruct *widget = (PMStruct*) myPMs->widget(tabNum);
    widget->state = PMStruct::NoMessages;
    myPMs->tabBar()->setTabTextColor(tabNum, myPMs->tabBar()->palette().text().color());
}

void PMSystem::togglePMs(bool toggled)
{
    tabbedPMs = toggled;
    changePMs();
}

void PMSystem::changePMs()
{
    if(!tabbedPMs) {
        while (myPMs->count() > 0) {
            myPMs->removeTab(0);
        }
        foreach(PMStruct *pm, myPMWindows) {
            pm->setWindowFlags(Qt::Window);
            pm->show();
        }
        hide();
    } else {
        foreach(PMStruct *pm, myPMWindows) {
            pm->hide();
            pm->setWindowFlags(Qt::Widget);
            myPMs->addTab(pm, pm->name());
        }
        if (myPMs->count() > 0) {
            show();
        }
    }
}

void PMSystem::changeId(int old, int newid)
{
    myPMWindows[newid] = myPMWindows.take(old);
}

void PMSystem::messageReceived(PMStruct *pm) {
    if (tabbedPMs) {
        if(isVisible()) {
            if(pm->state == PMStruct::NewMessage) {
                return;
            }
            if(pm == myPMs->currentWidget()) {
                return;
            }
            pm->state = PMStruct::NewMessage;
            for(int i = 0; i < myPMs->count(); i++) {
                if(myPMs->widget(i) == pm) {
                    myPMs->tabBar()->setTabTextColor(i, QColor(Qt::red));
                }
            }
        } else {
            show();
            for(int i = 0; i < myPMs->count(); i++) {
                if(myPMs->widget(i) == pm) {
                    myPMs->setCurrentIndex(i);
                }
            }
        }
    }
}

void PMSystem::PMDisconnected(bool value)
{
    foreach(PMStruct *pm, myPMWindows) {
        pm->disconnected(value);
    }
}

PMStruct::PMStruct(int id, const QString &ownName, const QString &name, const QString &content, bool html)
    : m_ownName(ownName), escape_html(!html)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->id() = id;
    changeName(name);
    SaveLog = false;

    QGridLayout *l = new QGridLayout(this);
    this->setLayout(l);

    m_mainwindow = new QScrollDownTextBrowser();
    m_textToSend = new QIRCLineEdit();

    l->addWidget(m_mainwindow, 0,0,1,2);
    l->addWidget(m_textToSend, 1,0,1,2);

    m_challenge = new QPushButton(tr("&Challenge"));
    m_send = new QPushButton(tr("&Ignore"));
    m_send->setCheckable(true);

    QSettings s;
    if(s.value("PMs/Logged").toBool()) {
        log = LogManager::obj()->createLog(PMLog, name + " -- " + ownName + " ");
        log->override = Log::OverrideYes;
        SaveLog = true;
    }

    l->addWidget(m_challenge,2,0);
    l->addWidget(m_send,2,1);

    printLine(content, false);

    connect(m_textToSend, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(m_send, SIGNAL(toggled(bool)), this, SLOT(ignore(bool)));
    connect(m_challenge, SIGNAL(clicked()), this, SLOT(challenge()));
}

void PMStruct::changeName(const QString &newname)
{
    this->m_name = newname;
    setWindowTitle(newname);
}

void PMStruct::changeSelf(const QString &newname)
{
    this->m_ownName = newname;
}

void PMStruct::printLine(QString line, bool self)
{
    if (line.trimmed().length() == 0)
        return;

    QSettings s;
    bool tt = s.value("PMs/ShowTimestamps").toBool();
    QString timeStr = "";

    if (tt)
        timeStr += "(" + QTime::currentTime().toString("hh:mm") + ") ";

    if (escape_html) {
        line = escapeHtml(line);
    }

    if (self) {
        printHtml(toColor(timeStr + "<b>" + escapeHtml(m_ownName) + ": </b>", Qt::darkBlue) + line, false);
    } else {
        printHtml(toColor(timeStr + "<b>" + escapeHtml(name()) + ": </b>", Qt::darkGray) + line, false);
    }
}

void PMStruct::printHtml(const QString &htmlCode, bool timestamps)
{
    QSettings s;
    bool tt = s.value("PMs/ShowTimestamps").toBool();
    QString timeStr = "";

    if (tt && timestamps)
        timeStr += "(" + QTime::currentTime().toString("hh:mm") + ") ";

    m_mainwindow->insertHtml(timeStr + removeTrollCharacters(htmlCode) + "<br />");
    if(SaveLog) {
        log->pushHtml(timeStr + removeTrollCharacters(htmlCode) + "<br />");
    }

    /* Dirty hack to not trigger events on player log off / log on, so that the pm window doesn't pop up
     if closed in those cases */
    if (m_challenge->isEnabled()) {
        emit messageReceived(this);
    }
}

void PMStruct::sendMessage()
{
    QString str = m_textToSend->text().trimmed();
    m_textToSend->clear();

    if (str.length() == 0) {
        return;
    }

    if (str.length() > 0) {
        QStringList s = str.split('\n');
        foreach(QString s1, s) {
            if (s1.length() > 0) {
                emit messageEntered(id(), s1);
                printLine(s1, true);
            }
        }
    }
}

void PMStruct::ignore(bool yes)
{
    emit ignore(id(), yes);
}

void PMStruct::challenge()
{
    emit challengeSent(id());
}

void PMStruct::disable()
{
    m_challenge->setDisabled(true);
    m_send->setDisabled(true);
    m_textToSend->setDisabled(true);

    printHtml("<i>" + tr("The other party left the server, so the window was disabled.") + "</i>");
}

void PMStruct::reuse(int id)
{
    if (this->id() == id) return;

    int oldid = this->id();

    this->id() = id;
    printHtml("<i>" + tr("The player has logged on again") + "</i>");
    m_challenge->setEnabled(true);
    m_send->setEnabled(true);
    m_textToSend->setEnabled(true);

    emit idChanged(oldid, id);
}

void PMStruct::disconnected(bool value)
{
    if(value) {
        printHtml("<i>" + tr("You've been disconnected from server.") + "</i>");
        m_challenge->setEnabled(false);
        m_send->setEnabled(false);
        m_textToSend->setEnabled(false);
    } else {
        printHtml("<i>" + tr("You've been reconnected to the server."), + "</i>");
        m_challenge->setEnabled(true);
        m_send->setEnabled(true);
        m_textToSend->setEnabled(true);
    }
}

void PMStruct::closeEvent(QCloseEvent *event) {
    log->close();
    event->accept();
}
