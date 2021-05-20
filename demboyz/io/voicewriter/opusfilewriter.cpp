#include <stdio.h>
#include <cassert>
#include <string.h>
#include "opusfilewriter.h"

/*
uint8_t header[16] = 'DEMOPUSHEADER_V2'
uint32_t sampleRate
uint64_t samples

while True:
    uint64_t samplesOfs
    uint32_t length
    uint8_t data[length]
*/

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
    assert(!m_OutFile);
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

    uint8_t customHeader[16] = {'D', 'E', 'M', 'O', 'P', 'U', 'S', 'H', 'E', 'A', 'D', 'E', 'R', '_', 'V', '2'};
    fwrite(customHeader, sizeof(customHeader), 1, m_OutFile);

    m_InfoStartPos = ftell(m_OutFile);
    fwrite(&m_SampleRate, sizeof(m_SampleRate), 1, m_OutFile);
    fwrite(&m_Samples, sizeof(m_Samples), 1, m_OutFile);

    m_Comments = ope_comments_create();

    SwitchState(STATE_SILENCE);
}

void OpusFileWriter::Close()
{
    if(!m_OutFile)
        return;

    SwitchState(STATE_DONE);

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

    if(m_State == STATE_SILENCE)
    {
        fwrite(&m_Samples, sizeof(m_Samples), 1, m_OutFile);
    }
    else if(m_State == STATE_OPUS)
    {
        ope_encoder_drain(m_Enc);
        ope_encoder_destroy(m_Enc);
        m_Enc = nullptr;

        long int dataEndPos = ftell(m_OutFile);
        fseek(m_OutFile, m_LengthStartPos, SEEK_SET);
        uint32_t dataLen = dataEndPos - (m_LengthStartPos + sizeof(uint32_t));
        fwrite(&dataLen, sizeof(dataLen), 1, m_OutFile);
        fseek(m_OutFile, dataEndPos, SEEK_SET);
    }

    if(newState == STATE_OPUS)
    {
        m_LengthStartPos = ftell(m_OutFile);
        uint32_t dataLen = 0;
        fwrite(&dataLen, sizeof(dataLen), 1, m_OutFile);

        int error;
        if(!m_Enc)
            m_Enc = ope_encoder_create_callbacks(&opus_callbacks, (void *)this, m_Comments, m_SampleRate, 1, 0, &error);
        assert(error == 0);
    }
    else if(newState == STATE_SILENCE)
    {
    }
    else if(newState == STATE_DONE)
    {
        long int backup = ftell(m_OutFile);
        fseek(m_OutFile, m_InfoStartPos, SEEK_SET);
        fwrite(&m_SampleRate, sizeof(m_SampleRate), 1, m_OutFile);
        fwrite(&m_Samples, sizeof(m_Samples), 1, m_OutFile);
        fseek(m_OutFile, backup, SEEK_SET);
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

    SwitchState(STATE_OPUS);

    ope_encoder_write(m_Enc, samples, numSamples);
    m_Samples += numSamples;
}

void OpusFileWriter::PadSilence(uint64_t milliseconds)
{
    uint64_t numSamples = (milliseconds * (uint64_t)m_SampleRate) / 1000UL;
    if(!m_OutFile || m_Samples >= numSamples)
        return;

    SwitchState(STATE_SILENCE);

    m_Samples = numSamples;
}
