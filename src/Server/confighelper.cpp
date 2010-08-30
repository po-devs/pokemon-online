#include "confighelper.h"
#include "../Utilities/otherwidgets.h"

/* I hate stupid GCC that forces to specify the base class
   of attributes when templates are involved, even though
   it can check if there's a problem or not and shut up
   when there's not.

   What the hell is undeclared when templates are compile-time?
   Do they know what lazy evaluation is?

   You can always bypass that using -fpermissive, but it's going to
   be deprecated -.-

   Visual C++ > GCC on this point */

AbstractConfigHelper::AbstractConfigHelper(const QString &desc) : description(desc) {

}

QWidget *AbstractConfigHelper::generateConfigWidget() {
    internalWidget = getInternalWidget();
    if (description.isEmpty()) {
        return internalWidget;
    } else {
        QWidget *w = new QWidget();
        w->setLayout(new QSideBySide(new QLabel(description), internalWidget));
        return w;
    }
}

void AbstractConfigHelper::editingDone()
{
    doWhenEditingDone();
}

template<class T>
ConfigHelper<T>::ConfigHelper(const QString &desc, T &var) : var(var) {

}


template<class T>
ConfigCombo<T>::ConfigCombo(const QString &desc, T &var, const QStringList &labels, const QList<T> &values)
    : ConfigHelper<T>(desc, var), labels(labels), values(values)
{

}

template<class T>
QWidget *ConfigCombo<T>::getInternalWidget()
{
    QComboBox *ret = new QComboBox();

    ret->addItems(labels);

    QObject::connect(ret, SIGNAL(activated(int)), SLOT(editingDone()));

    return ret;
}

template<class T>
void ConfigCombo<T>::doWhenEditingDone()
{
    int index = ((QComboBox*)(ConfigHelper<T>::internalWidget))->currentIndex();

    if (index >= 0 && index < values.size()) {
        ConfigHelper<T>::var = values[index];
    }
}
