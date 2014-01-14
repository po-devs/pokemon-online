#ifndef WAVREADER_H
#define WAVREADER_H

#include <QAudioFormat>

class QIODevice;

QAudioFormat readWavHeader(QIODevice *device);

#endif // WAVREADER_H
