#include <QIODevice>
#include <QtEndian>

#include "wavreader.h"

/*** WAV READER ***/

struct chunk
{
    char        id[4];
    quint32     size;
};

struct RIFFHeader
{
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};

struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};

struct DATAHeader
{
    chunk       descriptor;
};

struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
};

QAudioFormat readWavHeader(QIODevice *device)
{
    CombinedHeader header;
    bool result = device->read(reinterpret_cast<char *>(&header),
                               sizeof(CombinedHeader)) == sizeof(CombinedHeader);
    QAudioFormat format;

    if (result) {
        if ((memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
            || memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
            && memcmp(&header.riff.type, "WAVE", 4) == 0
            && memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
            && (header.wave.audioFormat == 1 || header.wave.audioFormat == 0)) {

            // Read off remaining header information
            DATAHeader dataHeader;

            if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
                // Extended data available
                quint16 extraFormatBytes;
                if (device->peek((char*)&extraFormatBytes, sizeof(quint16)) != sizeof(quint16))
                    return format;
                const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
                if (device->read(throwAwayBytes).size() != throwAwayBytes)
                    return format;
            }

            if (device->read((char*)&dataHeader, sizeof(DATAHeader)) != sizeof(DATAHeader))
                return format;

            // Establish format
            if (memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0)
                format.setByteOrder(QAudioFormat::LittleEndian);
            else
                format.setByteOrder(QAudioFormat::BigEndian);

            int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
            format.setChannelCount(qFromLittleEndian<quint16>(header.wave.numChannels));
            format.setCodec("audio/pcm");
            format.setSampleRate(qFromLittleEndian<quint32>(header.wave.sampleRate));
            format.setSampleSize(qFromLittleEndian<quint16>(header.wave.bitsPerSample));
            format.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        }
    }

    return format;
}

