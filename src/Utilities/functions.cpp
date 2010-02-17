#include "functions.h"
#include "md5.h"

QString escapeHtml(const QString & toConvert)
{
    QString ret = toConvert;

    ret.replace("&", "&amp;");
    ret.replace("<", "&lt;");
    ret.replace(">", "&gt;");

    return ret;
}

QString md5_hash(const QString &tohash) {
    QByteArray result = tohash.toUtf8();

    md5_state_t state;
    md5_init(&state);
    md5_append(&state, (const md5_byte_t*)result.constData(), result.length());

    md5_byte_t digest[16];
    md5_finish(&state, digest);

    QByteArray hexOutput;
    hexOutput.resize(32);

    for (int di = 0; di < 16; ++di)
        sprintf(hexOutput.begin() + di * 2, "%02x", digest[di]);
    return hexOutput;
}

void createIntMapper(QObject *src, const char *signal, QObject *dest, const char *slot, int id)
{
    QSignalMapper *mymapper = new QSignalMapper(src);
    mymapper->setMapping(src, id);
    src->connect(src, signal, mymapper, SLOT(map()));
    src->connect(mymapper, SIGNAL(mapped(int)), dest, slot);
}
