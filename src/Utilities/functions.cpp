#include "functions.h"

QString escapeHtml(const QString & toConvert)
{
    QString ret = toConvert;

    ret.replace("&", "&amp;");
    ret.replace("<", "&lt;");
    ret.replace(">", "&gt;");
    ret.replace(QRegExp("\\b(http://[^\\s]+)", Qt::CaseInsensitive), "<a href='\\1'>\\1</a>");

    return ret;
}

QByteArray md5_hash(const QByteArray &result) {
    return QCryptographicHash::hash(result, QCryptographicHash::Md5).toHex();
}

void createIntMapper(QObject *src, const char *signal, QObject *dest, const char *slot, int id)
{
    QSignalMapper *mymapper = new QSignalMapper(src);
    mymapper->setMapping(src, id);
    src->connect(src, signal, mymapper, SLOT(map()));
    src->connect(mymapper, SIGNAL(mapped(int)), dest, slot);
}

QString slug(const QString &s)
{
    QString ret = "";

    foreach(QChar c, s) {
        if (c.isLetter() || c == '_') {
            ret += c.toLower();
        }
    }

    return ret;
}
