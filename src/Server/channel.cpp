#include "../Utilities/otherwidgets.h"
#include "channel.h"

QNickValidator *Channel::checker = NULL;

Channel::Channel(const QString &name) : name(name), logDay(0) {
    QDir d("");
    if(!d.exists("logs/chat/" + name)) {
        d.mkpath("logs/chat/" + name);
    }
}

void Channel::log(const QString &message) {
    if(!logfile.isOpen() || logDay != QDate::currentDate().day()) {
        if(logfile.isOpen()) {
            logfile.close();
        }
        QString date = QDate::currentDate().toString("yyyy-MM-dd");
        QString filename = "logs/chat/"+name+"/"+date+".txt";
        logDay = QDate::currentDate().day();
        logfile.setFileName(filename);
        logfile.open(QFile::WriteOnly | QFile::Append | QFile::Text);
    }
    logfile.write(QString("(%1) %2\n").arg(QTime::currentTime().toString("hh:mm:ss"), message).toUtf8());
    logfile.flush();
}

bool Channel::validName(const QString &name) {
    return checker->validate(name) == QValidator::Acceptable;
}

Channel::~Channel()
{
    if(logfile.isOpen()) {
        logfile.close();
    }
}
