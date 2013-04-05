#include "functions.h"

#include <QtGui>

QString escapeHtml(const QString & toConvert)
{
    QString ret = toConvert;

    ret.replace("&", "&amp;");
    ret.replace("<", "&lt;");
    ret.replace(">", "&gt;");
    ret.replace(QRegExp("\\b((?:https?|ftp)://\\S+)", Qt::CaseInsensitive), "<a href='\\1'>\\1</a>");
    /* Revert &amp;'s to &'s in URLs */
    ret.replace(QRegExp("&amp;(?=[^\\s<]*</a>)"), "&");

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

QString cleanStringForFiles(const QString &title)
{
    QString ret = title;

    /* Those characters are banned in file names on windows */
    QList<QChar> bannedCh = QList<QChar> () << '"' << '/' << '\\' << ':' << '*' << '|' << '?' << '<' << '>';
    foreach(QChar c, bannedCh) {
        ret = ret.replace(c, ' ');
    }

    return ret;
}

void writeSettings(QWidget *w)
{
    QSettings settings;

    settings.beginGroup(w->metaObject()->className());
    settings.setValue("size", w->topLevelWidget()->size());
    settings.setValue("pos", w->topLevelWidget()->pos());
    settings.setValue("maximized", w->topLevelWidget()->isMaximized());
    settings.endGroup();
}

void loadSettings(QWidget *w, const QSize &defaultSize)
{
    QSettings settings;

    settings.beginGroup(w->metaObject()->className());
    if (settings.contains("size") || !defaultSize.isNull())
        w->topLevelWidget()->resize(settings.value("size", defaultSize).toSize());
//    if (settings.contains("pos")) {
//        QPoint pos = settings.value("pos").toPoint();
//        /* Checks if the position stored is not off the screen */
//        if (QApplication::desktop()->screenGeometry(pos).contains(pos)) {
//            w->topLevelWidget()->move(settings.value("pos").toPoint());
//        }
//    }
    if (settings.contains("maximized") && settings.value("maximized").toBool())
        w->topLevelWidget()->showMaximized();
    else
        w->topLevelWidget()->showNormal();

    settings.endGroup();
}

void cropImage(QImage &p)
{
    uchar * scanline = p.scanLine(0);

    uchar buffer[p.bytesPerLine()];
    memcpy(buffer, scanline, p.bytesPerLine());

    int c = 0;
    while (memcmp(p.scanLine(-c-1+p.height()), buffer, p.bytesPerLine()) == 0 && (-c-1+p.height() >= 0)) {
        c++;
    }

    for (int k = p.height() - 1; k - c >= 0; k--) {
        memcpy(p.scanLine(k), p.scanLine(k-c), p.bytesPerLine());
    }

    for (int k = 0; k < c; k++) {
        memcpy(p.scanLine(k), buffer, p.bytesPerLine());
    }
}

QString appDataPath(const QString &subfolder, bool createFolder)
{
    QString path = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + subfolder;

    if (createFolder) {
        QDir d;
        d.mkpath(path);
    }

    return path;
}

QString removeTrollCharacters(const QString& s)
{
    // All Non-Spacing Mark characters are banned and will trigger this filter.
    QString result = s;
    for (int x = 0;x<result.size();x++) {
        if ((result.at(x)).category() == QChar::Mark_NonSpacing || (result.at(x)) == QChar(0x202e)) {
            result.replace(result.at(x), "");
            x--;
        }
    }
    return result;
}

void removeFolder(const QString &path)
{
    QDir d(path);

    QStringList files = d.entryList(QDir::Files | QDir::Hidden | QDir::System);

    foreach(QString file, files) {
        d.remove(file);
    }

    QStringList dirs = d.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);

    foreach(QString dir, dirs) {
        removeFolder(d.absoluteFilePath(dir));
    }

    d.rmdir(d.absolutePath());
}


bool testWritable(const QString &f)
{
    QString path = f.isEmpty() ? QString("tmp%1").arg(rand()) : f;

    QFile w(path);

    if (!w.open(QIODevice::Append)) {
        return false;
    }

    if (!w.write("", 0)) {
        return false;
    }

    if (f.isEmpty()) {
        w.remove();
    }

    return true;
}
