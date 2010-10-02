#include "functions.h"

QString escapeHtml(const QString & toConvert)
{
    QString ret = toConvert;

    ret.replace("<", "&lt;");
    ret.replace(">", "&gt;");
    ret.replace(QRegExp("\\b((?:https?|ftp)://\\S+)", Qt::CaseInsensitive), "<a href='\\1'>\\1</a>");
    /* Don't replace amps within URLs */
    ret.replace(QRegExp("&(?!(?:[^\\s<]*</a>|[lg]t))"), "&amp;");

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
        if (c.isLetter() || c.isDigit() || c == '_') {
            ret += c.toLower();
        }
    }

    return ret;
}
