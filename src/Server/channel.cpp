#include "../Utilities/otherwidgets.h"
#include "channel.h"

QNickValidator *Channel::checker = NULL;

Channel::Channel(const QString &name) : name(name) {

}

bool Channel::validName(const QString &name) {
    return checker->validate(name) == QValidator::Acceptable;
}
