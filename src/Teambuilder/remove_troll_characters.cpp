#include "remove_troll_characters.h"

QString removeTrollCharacters(const QString& s)
{
    // All Non-Spacing Mark characters are banned and will trigger this filter.
    QString result = s;
    QChar const rtloverride(0x202e);
    for (int x = 0;x<result.size();x++) {
        if ((result.at(x)).category() == QChar::Mark_NonSpacing) {
            result.replace(result.at(x), "");
            x = 0;
        }
    }
    // Removes RTL override
    if (result.contains(rtloverride)) {
        result.replace(rtloverride, "");
    }
    return result;
}
