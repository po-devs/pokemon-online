#include <QDomElement>
#include <QDomDocument>
#include <QMessageBox>
#include "Teambuilder/teamholder.h"

QStringList Profile::getProfileList(const QString &path)
{
    QDir profilesPath(path);
    QStringList profilesList;
    foreach(const QString &name, profilesPath.entryList(QStringList("*.xml"))) {
        profilesList.append(QUrl::fromPercentEncoding(name.section(".", 0, -2).toUtf8()));
    }
    return profilesList;
}

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

    qDebug() << "name: " << name();

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
    const_cast<Profile*>(this)->sanitize();
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

void Profile::deleteProfile(const QString &path)
{
    QFile file(path);
    if(file.isOpen()) {
        QMessageBox::warning(0, QObject::tr("Deleting Profile"), QObject::tr("Couldn't delete profile: %1\n%2").arg(file.fileName(), file.errorString()));
        return;
    }
    file.remove();
}

void Profile::sanitize()
{
    name().resize(std::min(name().length(), 20));
    info().sanitize();
}

TeamHolder::TeamHolder()
{
    m_teams.push_back(Team());
    m_currentTeam = 0;
}

const Team &TeamHolder::team() const
{
    return m_teams[currentTeam()];
}

Team &TeamHolder::team()
{
    return m_teams[currentTeam()];
}

const Team &TeamHolder::team(int i) const
{
    return m_teams[i];
}

Team &TeamHolder::team(int i)
{
    return m_teams[i];
}

int TeamHolder::officialCount() const
{
    if (m_tiers.isEmpty())
        return m_teams.size();
    else
        return std::min(m_tiers.size(), m_teams.size());
}

int TeamHolder::count() const
{
    return m_teams.size();
}

QString TeamHolder::tier() const
{
    return tier(currentTeam());
}

QString TeamHolder::tier(int team) const
{
    return team < m_tiers.size() ? m_tiers[team] : QObject::tr("No Tier");
}

void TeamHolder::setTiers(const QStringList &tiers)
{
    m_tiers = tiers;
}

void TeamHolder::save()
{
    QSettings s;

    QStringList locations;

    for (int i = 0; i < count(); i++) {
        if (team(i).name().isEmpty()) {
            QMessageBox::warning(NULL, QObject::tr("Impossible to save team"),
                                 QObject::tr("The team number %1 could not be saved as it was given no name!").arg(i+1));
        } else {
            team(i).saveToFile(team(i).path());
            locations.push_back(team(i).path());
        }
    }

    s.setValue("Teams/Locations", locations);
    QString path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(name()) + ".xml";
    profile().saveToFile(path);
}

void TeamHolder::load()
{
    QSettings s;

    m_teams.clear();
    if (!s.contains("Teams/Locations")) {
        addTeam();
        setCurrent(0);
    } else {
        QStringList l = s.value("Teams/Locations").toStringList();

        for (int i = 0; i < l.length(); i++) {
            addTeam();
            team(i).loadFromFile(l[i]);
        }

        if (count() == 0) {
            addTeam();
            setCurrent(0);
        } else {
            if (currentTeam() >= count()) {
                setCurrent(count()-1);
            }
        }
    }

    if (!team().path().isEmpty()) {
        s.setValue("Teams/Folder", team().folder());
    }

    profile().loadFromFile(s.value("Profile/Current").toString());
}

void TeamHolder::addTeam()
{
    m_teams.push_back(Team());

    if (currentTeam() < count() - 1 && !team(currentTeam()).folder().isEmpty()) {
        m_teams.back().setFolder(team(currentTeam()).folder());
    } else {
        QSettings s;

        m_teams.back().setFolder(s.value("Teams/Folder").toString());
    }
}

void TeamHolder::removeTeam()
{
    if (count() > 1) {
        m_teams.removeAt(currentTeam());
    }

    if (currentTeam() >= count()) {
        m_currentTeam -= 1;
    }
}
