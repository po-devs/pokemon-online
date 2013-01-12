#ifndef TEAMBUILDERINTERFACE_H
#define TEAMBUILDERINTERFACE_H

class TeambuilderPlugin;

class TeambuilderInterface {
public:
    virtual ~TeambuilderInterface(){}

    virtual void addPlugin(TeambuilderPlugin *o) = 0;
    virtual void removePlugin(TeambuilderPlugin *o) = 0;
};

#endif // TEAMBUILDERINTERFACE_H
