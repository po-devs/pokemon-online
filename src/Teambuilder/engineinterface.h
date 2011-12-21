#ifndef PLUGINDELEGATE_H
#define PLUGINDELEGATE_H

class TrainerTeam;
class ThemeAccessor;

class MainEngineInterface {
public:
    virtual TrainerTeam* trainerTeam() = 0;
    virtual ThemeAccessor* theme() = 0;
};

#endif // PLUGINDELEGATE_H
