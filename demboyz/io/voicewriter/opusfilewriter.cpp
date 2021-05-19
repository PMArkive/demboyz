#include <stdio.h>
#include <cassert>
#include <string.h>
#include "opusfilewriter.h"


int opus_stream_write(void *user_data, const unsigned char *ptr, opus_int32 len)
{
    OpusFileWriter *pThis = (OpusFileWriter *)user_data;
    return pThis->opus_stream_write(ptr, len);
}

int opus_stream_close(void *user_data)
{
    OpusFileWriter *pThis = (OpusFileWriter *)user_data;
    return pThis->opus_stream_close();
}

static OpusEncCallbacks opus_callbacks = {
    opus_stream_write,
    opus_stream_close
};


OpusFileWriter::OpusFileWriter()
{
}

OpusFileWriter::~OpusFileWriter()
{
    assert(!m_Enc);
}

void OpusFileWriter::Init(const char* file, uint32_t sampleRate)
{
    assert(!m_OutFile);
    m_SampleRate = sampleRate;
    m_Samples = 0;

    m_OutFile = fopen(file, "wb");
    if(!m_OutFile)
    {
        fprintf(stderr, "Error: Could not open %s\n", file);
        return;
    }

    uint8_t customHeader[16] = {'D', 'E', 'M', 'O', 'P', 'U', 'S', 'H', 'E', 'A', 'D', 'E', 'R', '_', 'V', '1'};
    fwrite(customHeader, sizeof(customHeader), 1, m_OutFile);

    uint8_t header = HEADER_TYPE_INFO;
    fwrite(&header, sizeof(header), 1, m_OutFile);
    m_InfoStartPos = ftell(m_OutFile);
    fwrite(&m_SampleRate, sizeof(m_SampleRate), 1, m_OutFile);
    fwrite(&m_Samples, sizeof(m_Samples), 1, m_OutFile);

    m_Comments = ope_comments_create();
}

void OpusFileWriter::Close()
{
    if(!m_OutFile)
        return;

    SwitchState(HEADER_TYPE_INFO);
    SwitchState(HEADER_TYPE_DONE);

    ope_comments_destroy(m_Comments);
    m_Enc = nullptr;
    m_Comments = nullptr;
    fclose(m_OutFile);
    m_OutFile = nullptr;
}

void OpusFileWriter::SwitchState(int newState)
{
    if(!m_OutFile)
        return;

    if(m_State == newState)
        return;

    if(m_State == HEADER_TYPE_SILENCE)
    {
        fwrite(&m_SilenceSamples, sizeof(m_SilenceSamples), 1, m_OutFile);
    }
    else if(m_State == HEADER_TYPE_OPUS)
    {
        ope_encoder_drain(m_Enc);
        ope_encoder_destroy(m_Enc);
        m_Enc = nullptr;

        m_DataEndPos = ftell(m_OutFile);
        fseek(m_OutFile, m_LengthStartPos, SEEK_SET);
        uint64_t dataLen = m_DataEndPos - (m_LengthStartPos + sizeof(uint64_t));
        fwrite(&dataLen, sizeof(dataLen), 1, m_OutFile);
        fseek(m_OutFile, m_DataEndPos, SEEK_SET);
    }

    if(newState == HEADER_TYPE_OPUS)
    {
        uint8_t header = HEADER_TYPE_OPUS;
        fwrite(&header, sizeof(header), 1, m_OutFile);
        m_LengthStartPos = ftell(m_OutFile);
        uint64_t dataLen = 0;
        fwrite(&dataLen, sizeof(dataLen), 1, m_OutFile);

        int error;
        if(!m_Enc)
            m_Enc = ope_encoder_create_callbacks(&opus_callbacks, (void *)this, m_Comments, m_SampleRate, 1, 0, &error);
        assert(error == 0);
    }
    else if(newState == HEADER_TYPE_SILENCE)
    {
        m_SilenceStart = m_Samples;
        m_SilenceSamples = 0;

        uint8_t header = HEADER_TYPE_SILENCE;
        fwrite(&header, sizeof(header), 1, m_OutFile);
    }
    else if(newState == HEADER_TYPE_INFO)
    {
        long int backup = ftell(m_OutFile);
        fseek(m_OutFile, m_InfoStartPos, SEEK_SET);
        fwrite(&m_SampleRate, sizeof(m_SampleRate), 1, m_OutFile);
        fwrite(&m_Samples, sizeof(m_Samples), 1, m_OutFile);
        fseek(m_OutFile, backup, SEEK_SET);
    }
    else if(newState == HEADER_TYPE_DONE)
    {
        uint8_t header = HEADER_TYPE_DONE;
        fwrite(&header, sizeof(header), 1, m_OutFile);
    }

    m_State = newState;
}

int OpusFileWriter::opus_stream_write(const unsigned char *ptr, opus_int32 len)
{
    assert(fwrite(ptr, len, 1, m_OutFile) != (size_t)len);
    return 0;
}

int OpusFileWriter::opus_stream_close()
{
    return 0;
}

void OpusFileWriter::WriteSamples(const int16_t* samples, uint32_t numSamples)
{
    if(!m_OutFile)
        return;

    SwitchState(HEADER_TYPE_OPUS);

    ope_encoder_write(m_Enc, samples, numSamples);
    m_Samples += numSamples;
}

void OpusFileWriter::PadSilence(uint64_t milliseconds)
{
    uint64_t numSamples = (milliseconds * (uint64_t)m_SampleRate) / 1000UL;
    if(!m_OutFile || m_Samples >= numSamples)
        return;

    SwitchState(HEADER_TYPE_SILENCE);

    m_Samples = numSamples;
    m_SilenceSamples = m_Samples - m_SilenceStart;
}
