#include <QDomElement>
#include <QDomDocument>
#include <QMessageBox>
#include "teamholder.h"

bool Profile::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }
    QDomDocument document;
    QString msg;
    int line,col;
    if(!document.setContent(&file,&msg,&line,&col))
    {
        QMessageBox::information(0,QObject::tr("Loading Profile"),QObject::tr("Error while loading the profile: %1 (line %2, col %3).").arg(msg).arg(line).arg(col));
        return false;
    }
    QDomElement team = document.firstChildElement("profile");
    if(team.isNull())
    {
        QMessageBox::information(0,QObject::tr("Loading Profile"),QObject::tr("Error while loading the profile."));
        return false;
    }
    int version = team.attribute("version", "1").toInt();

    if (version > 1) {
        QMessageBox::information(0,QObject::tr("Loading Profile"),QObject::tr("Error while loading the profile, the client is outdated."));
        return false;
    }

    name() = team.attribute("name");
    if (team.hasAttribute("color")) {
        color() = QColor(team.attribute("color"));
    } else {
        color() = QColor();
    }
    info().avatar = team.attribute("avatar", "1").toInt();
    info().winning = team.attribute("winningMessage");
    info().losing = team.attribute("losingMessage");
    info().tie = team.attribute("tieMessage");
    info().info = team.attribute("information");

    return true;
}

void Profile::toXml(QDomDocument &document) const
{
    QDomElement profile = document.createElement("profile");
    profile.setAttribute("version", 1);
    profile.setAttribute("name", name());
    if (color().isValid()) {
        profile.setAttribute("color", color().name());
    }
    profile.setAttribute("avatar", info().avatar);
    profile.setAttribute("information", info().info);
    profile.setAttribute("winningMessage", info().winning);
    profile.setAttribute("tieMessage", info().tie);
    profile.setAttribute("losingMessage", info().losing);

    document.appendChild(profile);
}

QString Profile::toXml() const
{
    QDomDocument document;

    toXml(document);

    return document.toString();
}

bool Profile::saveToFile(const QString &path) const
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0, QObject::tr("Error while saving the profile"),QObject::tr("Can't create file %1").arg(file.fileName()));
        return false;
    }

    QDomDocument document;

    toXml(document);

    QTextStream in(&file);
    document.save(in,4);
    return true;
}

void TeamHolder::save()
{
    QSettings s;
    team().saveToFile(s.value("team_location").toString());
    profile().saveToFile(s.value("profile_location").toString());
}

void TeamHolder::load()
{
    QSettings s;
    team().loadFromFile(s.value("team_location").toString());
    profile().loadFromFile(s.value("profile_location").toString());
}
