#pragma once

#include <cstdint>
#include <opusenc.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum {
    HEADER_TYPE_NONE = 0,
    HEADER_TYPE_INFO = 0x01,
    HEADER_TYPE_OPUS = 0x02,
    HEADER_TYPE_SILENCE = 0x03,
    HEADER_TYPE_DONE = 0x04
};

class OpusFileWriter
{
public:
    OpusFileWriter();
    ~OpusFileWriter();
    void Init(const char* file, uint32_t sampleRate);
    void Close();
    void SwitchState(int newState);

    void WriteSamples(const int16_t* samples, uint32_t numSamples);
    void PadSilence(uint64_t milliseconds);

    int opus_stream_write(const unsigned char *ptr, opus_int32 len);
    int opus_stream_close();
private:
    FILE *m_OutFile = nullptr;
    int m_State = HEADER_TYPE_NONE;
    long int m_InfoStartPos;
    long int m_LengthStartPos;
    long int m_DataEndPos;

    uint64_t m_SilenceStart = 0;
    uint64_t m_SilenceSamples = 0;

    OggOpusComments *m_Comments = nullptr;
    OggOpusEnc *m_Enc = nullptr;
    uint32_t m_SampleRate = 0;
    uint64_t m_Samples = 0;

    static const uint32_t bytesPerSample = 2;
};
