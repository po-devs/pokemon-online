#ifndef PLUGINDELEGATE_H
#define PLUGINDELEGATE_H

class TeamHolderInterface;
class QScrollDownTextBrowser;
class QWidget;
struct ThemeAccessor;

class MainEngineInterface {
public:
    virtual TeamHolderInterface* trainerTeam() = 0;
    virtual ThemeAccessor* theme() = 0;
    virtual QScrollDownTextBrowser* getPokeTextEdit(QWidget* parent=0) = 0;
};

#endif // PLUGINDELEGATE_H
