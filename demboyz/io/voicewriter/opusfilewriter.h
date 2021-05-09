
#pragma once

#include <stdio.h>
#include <cassert>
#include <cstdint>
#include <opusenc.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

class OpusFileWriter
{
public:
    OpusFileWriter()
    {
    }

    ~OpusFileWriter()
    {
        assert(!m_Enc);
    }

    void Init(const char* file, uint32_t sampleRate)
    {
        assert(!m_Enc);
        m_SampleRate = sampleRate;
        m_Samples = 0;

        m_Comments = ope_comments_create();

        int error;
        m_Enc = ope_encoder_create_file(file, m_Comments, sampleRate, 1, 0, &error);
        assert(error == 0);

        ope_encoder_ctl(m_Enc, OPUS_SET_DTX(1));
    }

    void Close()
    {
        if(!m_Enc)
            return;

        ope_encoder_drain(m_Enc);
        ope_encoder_destroy(m_Enc);
        ope_comments_destroy(m_Comments);
        m_Enc = nullptr;
        m_Comments = nullptr;
    }

    void WriteSamples(const int16_t* samples, uint32_t numSamples)
    {
        if(!m_Enc)
            return;

        ope_encoder_write(m_Enc, samples, numSamples);
        m_Samples += numSamples;
    }

    void PadSilence(uint64_t milliseconds)
    {
        uint64_t numSamples = (milliseconds * (uint64_t)m_SampleRate) / 1000UL;
        if(!m_Enc || m_Samples >= numSamples)
            return;

        static const int16_t silence[128] = {0};
        uint64_t pad = numSamples - m_Samples;
        while(pad > 0)
        {
            const int samples = MIN(sizeof(silence) / bytesPerSample, pad);
            ope_encoder_write(m_Enc, silence, samples);
            pad -= samples;
        }
        m_Samples = numSamples;
    }

private:
    OggOpusComments *m_Comments = nullptr;
    OggOpusEnc *m_Enc = nullptr;
    uint32_t m_SampleRate = 0;
    uint64_t m_Samples = 0;

    static const uint32_t bytesPerSample = 2;
};
