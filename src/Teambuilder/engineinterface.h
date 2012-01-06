#ifndef PLUGINDELEGATE_H
#define PLUGINDELEGATE_H

class TeamHolder;
class ThemeAccessor;

class MainEngineInterface {
public:
    virtual TeamHolder* trainerTeam() = 0;
    virtual ThemeAccessor* theme() = 0;
};

#endif // PLUGINDELEGATE_H
