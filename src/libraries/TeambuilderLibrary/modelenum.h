#ifndef MODELENUM_H
#define MODELENUM_H

#include <Qt>

namespace CustomModel {
    enum {
        PokenumRole = Qt::UserRole+1,
        PokenameRole,
        PokeimageRole,
        MovenumRole,
        PokegenRole
    };
}
#endif // MODELENUM_H
