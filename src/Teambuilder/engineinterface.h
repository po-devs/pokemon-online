#ifndef PLUGINDELEGATE_H
#define PLUGINDELEGATE_H

class TrainerTeam;

class MainEngineInterface {
public:
    virtual TrainerTeam* trainerTeam() = 0;
};

#endif // PLUGINDELEGATE_H
