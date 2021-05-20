#pragma once

#include <cstdint>
#include <opusenc.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum {
    STATE_DONE = 0,
    STATE_SILENCE = 1,
    STATE_OPUS = 2
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
    int m_State = STATE_DONE;
    long int m_InfoStartPos;
    long int m_LengthStartPos;
    long int m_SilenceStartPos;

    OggOpusComments *m_Comments = nullptr;
    OggOpusEnc *m_Enc = nullptr;
    uint32_t m_SampleRate = 0;
    uint64_t m_Samples = 0;

    static const uint32_t bytesPerSample = 2;
};
