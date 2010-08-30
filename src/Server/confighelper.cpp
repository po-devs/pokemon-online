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

void ConfigForm::addConfigHelper(AbstractConfigHelper *helper)
{
    helpers.push_back(helper);
}

QWidget* ConfigForm::generateConfigWidget()
{
    QWidget *ret = new QWidget();

    QVBoxLayout *v = new QVBoxLayout(ret);

    foreach(AbstractConfigHelper *helper, helpers) {
        v->addWidget(helper->generateConfigWidget());
    }

    QPushButton *cancel = new QPushButton("Cancel");
    QPushButton *apply = new QPushButton("Apply");

    v->addLayout(new QSideBySide(cancel, apply));

    return ret;
}

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

    for (int i = 0; i < values.size(); i++) {
        if (values[i] == ConfigHelper<T>::var) {
            ret->setCurrentIndex(i);
            break;
        }
    }

    QObject::connect(ret, SIGNAL(activated(int)), SLOT(editingDone()));

    return ret;
}

template<class T>
void ConfigCombo<T>::updateVal()
{
    int index = ((QComboBox*)(ConfigHelper<T>::internalWidget))->currentIndex();

    if (index >= 0 && index < values.size()) {
        ConfigHelper<T>::var = values[index];
    }
}

ConfigSpin::ConfigSpin(const QString &desc, int &var, int min, int max)
    : ConfigHelper<int> (desc, var), min(min), max(max)
{

}

QWidget * ConfigSpin::getInternalWidget()
{
    QSpinBox *ret = new QSpinBox();

    ret->setRange(min, max);
    ret->setValue(var);

    return ret;
}

void ConfigSpin::updateVal()
{
    var = ((QSpinBox*)(ConfigHelper<T>::internalWidget))->value();
}

ConfigLine::ConfigLine(const QString &desc, QString &var)
    : ConfigHelper<QString> (desc, var)
{

}

void ConfigLine::updateVal()
{
    var = ((QLineEdit*)(ConfigHelper<T>::internalWidget))->text();
}

QWidget *ConfigLine::getInternalWidget()
{
    QLineEdit *ret = new QLineEdit();
    ret->setText(var);

    return ret;
}

ConfigCheck::ConfigCheck(const QString &desc, bool &var)
    : ConfigHelper<bool> ("", var), checkBoxText(desc)
{

}

void ConfigCheck::updateVal()
{
    var = ((QCheckBox*)(ConfigHelper<T>::internalWidget))->isChecked();
}

QWidget *ConfigCheck::getInternalWidget()
{
    QCheckBox *ret = new QCheckBox(checkBoxText);
    ret->setChecked(var);

    return ret;
}
