#include "netmessages/netmessages.h"

#include "netmessages/svc_voiceinit.h"
#include "netmessages/svc_voicedata.h"

#include <celt/celt.h>
#include <SKP_Silk_SDK_API.h>

#include <cassert>

#include "base/CRC.h"
#include "game/logic.h"
#include "voicedatawriter.h"
#include <cstring>

struct CeltConfig
{
    celt_int32 sampleRate;
    uint32_t frameSizeSamples;
    uint32_t encodedFrameSizeBytes;
};

static CeltConfig sCeltConfigs[] =
{
    { 44100, 256, 120 },    // unused
    { 22050, 120, 60 },     // unused
    { 22050, 256, 60 },     // unused
    { 22050, 512, 64 },     // vaudio_celt
    { 44100, 1024, 128 }    // vaudio_celt_high
};

bool CeltVoiceDecoder::DoInit(CELTMode* celtMode, uint32_t frameSizeSamples, uint32_t encodedFrameSizeBytes)
{
    if(m_celtDecoder)
    {
        return false;
    }

    int error = CELT_OK;
    m_celtDecoder = celt_decoder_create_custom(celtMode, sCeltChannels, &error);
    assert(error == CELT_OK);
    assert(m_celtDecoder);

    m_frameSizeSamples = frameSizeSamples;
    m_encodedFrameSizeBytes = encodedFrameSizeBytes;
    return true;
}

void CeltVoiceDecoder::Destroy()
{
    celt_decoder_destroy(m_celtDecoder);
    m_celtDecoder = NULL;
}

void CeltVoiceDecoder::Reset()
{
}

int CeltVoiceDecoder::Decompress(
    const uint8_t* compressedData,
    uint32_t compressedBytes,
    int16_t* uncompressedData,
    uint32_t maxUncompressedSamples)
{
    uint32_t curCompressedByte = 0;
    uint32_t curDecompressedSample = 0;

    const uint32_t encodedframeSizeBytes = m_encodedFrameSizeBytes;
    const uint32_t frameSizeSamples = m_frameSizeSamples;
    while(
        ((compressedBytes - curCompressedByte) >= encodedframeSizeBytes) &&
        ((maxUncompressedSamples - curDecompressedSample) >= frameSizeSamples))
    {
        DecodeFrame(&compressedData[curCompressedByte], &uncompressedData[curDecompressedSample]);
        curCompressedByte += encodedframeSizeBytes;
        curDecompressedSample += frameSizeSamples;
    }
    return curDecompressedSample;
}

void CeltVoiceDecoder::DecodeFrame(const uint8_t* compressedData, int16_t* uncompressedData)
{
    int error = celt_decode(m_celtDecoder, compressedData, m_encodedFrameSizeBytes, uncompressedData, m_frameSizeSamples);
    assert(error >= CELT_OK);
}


bool SilkVoiceDecoder::DoInit(int32_t sampleRate)
{
    if(m_Silk_DecoderState)
    {
        return false;
    }
    m_Silk_DecoderControl.API_sampleRate = sampleRate;

    int decoderSize;
	SKP_Silk_SDK_Get_Decoder_Size(&decoderSize);

    m_Silk_DecoderState = malloc(decoderSize);
    assert(m_Silk_DecoderState != NULL);

    int retEnc = SKP_Silk_SDK_InitDecoder(m_Silk_DecoderState);
    assert(retEnc == SKP_SILK_NO_ERROR);

    return true;
}

void SilkVoiceDecoder::Destroy()
{
    if(m_Silk_DecoderState)
        free(m_Silk_DecoderState);
    m_Silk_DecoderState = NULL;
}

void SilkVoiceDecoder::Reset()
{
    SKP_Silk_SDK_InitDecoder(m_Silk_DecoderState);
}

int SilkVoiceDecoder::Decompress(
    const uint8_t* compressedData,
    uint32_t compressedBytes,
    int16_t* uncompressedData,
    uint32_t maxUncompressedSamples)
{
    short nSamplesOut = maxUncompressedSamples;
    int decodeRes = SKP_Silk_SDK_Decode(m_Silk_DecoderState, &m_Silk_DecoderControl, 0, compressedData, compressedBytes, uncompressedData, &nSamplesOut);

    if (SKP_SILK_NO_ERROR != decodeRes)
        return 0;
    return nSamplesOut;
}


VoiceDataWriter::VoiceDataWriter(SourceGameContext* context, const char* outputPath):
    context(context),
    m_outputPath(outputPath)
{
}

void VoiceDataWriter::Start()
{
    int error = CELT_OK;
    const CeltConfig& config = sCeltConfigs[sQuality];
    m_celtMode = celt_mode_create(config.sampleRate, config.frameSizeSamples, &error);
    assert(error == CELT_OK);
    assert(m_celtMode);
}

void VoiceDataWriter::Finish()
{
    if(m_isSilenced)
    {
        m_isSilenced = false;
        m_silence.back().second = m_curTick;
    }

    for(auto& state : m_playerVoiceStates)
    {
        state.second.celt_decoder.Destroy();
        state.second.silk_decoder.Destroy();

        state.second.fileWriter.PadSilence((((uint64_t)m_curTick - m_silenceTicks) * 1000000UL) / (uint64_t)(context->fTickRate * 1000UL));
        state.second.fileWriter.Close();
        state.second.lastVoiceDataTick = -1;
    }

    if(m_celtMode)
    {
        celt_mode_destroy(m_celtMode);
        m_celtMode = nullptr;
    }
}

void VoiceDataWriter::StartCommandPacket(const CommandPacket& packet)
{
    m_lastTick = m_curTick;
    m_curTick = packet.tick;
}

void VoiceDataWriter::EndCommandPacket(const PacketTrailingBits& trailingBits)
{
    const int tickMargin = context->fTickInterval * 150.0; // ms
    if (m_curTick <= tickMargin)
        return;

    // Skip silence if noone talks for at least 3 seconds
    if((m_curTick - m_lastVoiceTick) / context->fTickRate > 3.0)
    {
        if(!m_isSilenced)
        {
            m_isSilenced = true;
            m_silence.emplace_back(std::make_pair(m_curTick, 0));
        }
        m_silenceTicks += (m_curTick - m_lastTick);
    }
    else if(m_isSilenced)
    {
        m_isSilenced = false;
        m_silence.back().second = m_curTick;
    }

    for(auto& state : m_playerVoiceStates)
    {
        if((m_curTick - state.second.lastVoiceDataTick) > tickMargin)
            state.second.fileWriter.PadSilence((((uint64_t)m_curTick - m_silenceTicks) * 1000000UL) / (uint64_t)(context->fTickRate * 1000UL));
    }
}

int VoiceDataWriter::ParseSteamVoicePacket(const uint8_t* bytes, int numBytes, PlayerVoiceState& state)
{
    int numDecompressedSamples = 0;
    int pos = 0;
    if(numBytes < 4+4+4+1+2)
        return -1;

    int dataLen = numBytes - 4; // skip CRC

    uint32_t CRCdemo = *((uint32_t *)&bytes[dataLen]);
    uint32_t CRCdata = CRC::Calculate(bytes, dataLen, CRC::CRC_32());
    if(CRCdata != CRCdemo)
        return -1;

    //uint32_t iSteamAccountID = *((uint32_t *)&bytes[pos]);
    pos += 4;
    uint32_t iSteamCommunity = *((uint32_t *)&bytes[pos]);
    pos += 4;

    if(iSteamCommunity != 0x1100001)
        return -1;

    while (pos < dataLen)
    {
        uint8_t payloadType = bytes[pos];
        pos++;

        switch(payloadType)
        {
            case 11: // Sample Rate
            {
                if(pos + 2 > dataLen)
                    return numDecompressedSamples;
                uint16_t rate = *((int16_t *)&bytes[pos]);
                pos += 2;
                if(state.silk_decoder.DoInit(rate))
                    state.sampleRate = rate;
            } break;
            case 10: // Unknown / Unused
            {
                if(pos + 2 > dataLen)
                    return numDecompressedSamples;
                //short unk = *((int16_t *)&bytes[pos]);
                pos += 2;
            } break;
            case 1: // Unknown Codec???
            case 2: // Speex Data (Unsupported)
            case 3: // Uncompressed Data
            case 4: // SILK Data
            {
                if(pos + 2 > dataLen)
                    return numDecompressedSamples;
                uint16_t length = *((int16_t *)&bytes[pos]);
                pos += 2;

                if(pos + length > dataLen)
                    return numDecompressedSamples;

                int freeSamples = (sizeof(m_decodeBuffer) / sizeof(int16_t)) - numDecompressedSamples;
                if(payloadType == 3)
                {
                    length = MIN(freeSamples * 2, length);
                    memcpy(&m_decodeBuffer[numDecompressedSamples], &bytes[pos], length);
                    numDecompressedSamples += length / sizeof(int16_t);
                }
                else if(payloadType == 4)
                {
                    int tpos = pos;
                    int maxpos = tpos + length;
                    while(tpos <= (maxpos - 2))
                    {
                        int16_t chunkLength = *((int16_t *)&bytes[tpos]);
                        tpos += 2;

                        if(chunkLength < 0)
                        {
                            state.silk_decoder.Reset();
                            continue;
                        }
                        else if(chunkLength == 0)
                        {
                            // DTX (discontinued transmission)
                            int numEmptySamples = state.sampleRate / 50;
                            numEmptySamples = MIN(freeSamples, numEmptySamples);
                            memset(&m_decodeBuffer[numDecompressedSamples], 0, numEmptySamples * sizeof(int16_t));
                            numDecompressedSamples += numEmptySamples;
                            continue;
                        }

                        if(tpos + chunkLength > maxpos)
                            return numDecompressedSamples;

                        int ret = state.silk_decoder.Decompress(&bytes[tpos], chunkLength, &m_decodeBuffer[numDecompressedSamples], freeSamples);
                        numDecompressedSamples += ret;
                        tpos += chunkLength;
                    }
                }
                pos += length;

            } break;
            case 0: // Silence
            {
                if(pos + 2 > dataLen)
                    return numDecompressedSamples;
                uint16_t numSamples = *((uint16_t *)&bytes[pos]);
                int freeSamples = (sizeof(m_decodeBuffer) / sizeof(int16_t)) - numDecompressedSamples;
                numSamples = MIN(freeSamples, numSamples);
                memset(&m_decodeBuffer[numDecompressedSamples], 0, numSamples * sizeof(int16_t));
                numDecompressedSamples += numSamples;
                pos += 2;
            } break;
        }
    }

    return numDecompressedSamples;
}

void VoiceDataWriter::OnNetPacket(NetPacket& packet)
{
    if(packet.type == NetMsg::svc_VoiceInit)
    {
        NetMsg::SVC_VoiceInit* voiceInit = static_cast<NetMsg::SVC_VoiceInit*>(packet.data);
        if(!strcmp(voiceInit->voiceCodec, "vaudio_celt"))
        {
            assert(voiceInit->quality == NetMsg::SVC_VoiceInit::QUALITY_HAS_SAMPLE_RATE);
            assert(voiceInit->sampleRate == sCeltConfigs[sQuality].sampleRate);
            m_Codec = CODEC_CELT;
        }
        else if(!strcmp(voiceInit->voiceCodec, "vaudio_speex"))
        {
            m_Codec = CODEC_SPEEX;
        }
        else //if(!strcmp(voiceInit->voiceCodec, "steam"))
        {
            m_Codec = CODEC_STEAM;
        }
        context->logic->OnVoiceCodec(voiceInit->voiceCodec, voiceInit->quality, voiceInit->sampleRate);
    }
    else if(packet.type == NetMsg::svc_VoiceData)
    {
        NetMsg::SVC_VoiceData* voiceData = static_cast<NetMsg::SVC_VoiceData*>(packet.data);
        assert(voiceData->fromClientIndex < MAX_PLAYERS);
        const char* guid = context->players[voiceData->fromClientIndex].info.guid;

        const uint8_t* bytes = voiceData->data.get();
        assert((voiceData->dataLengthInBits % 8) == 0);
        const int numBytes = voiceData->dataLengthInBits / 8;

        int numDecompressedSamples = 0;

        PlayerVoiceState& state = m_playerVoiceStates[guid];

        if(m_Codec == CODEC_CELT)
        {
            const CeltConfig& config = sCeltConfigs[sQuality];
            state.sampleRate = config.sampleRate;

            state.celt_decoder.DoInit(m_celtMode, config.frameSizeSamples, config.encodedFrameSizeBytes);
            numDecompressedSamples = state.celt_decoder.Decompress(bytes, numBytes, m_decodeBuffer, sizeof(m_decodeBuffer) / sizeof(int16_t));
        }
        else
        {
            // Try Steam Voice
            if(numBytes >= 15)
            {
                numDecompressedSamples = ParseSteamVoicePacket(bytes, numBytes, state);
            }

            // Would try speex here, if I cared...
            if(numDecompressedSamples <= 0)
            {
            }
        }

        if(numDecompressedSamples <= 0)
            return;

        if(state.lastVoiceDataTick == -1)
        {
            std::string name = std::string(m_outputPath) + "/" + std::string(guid) + ".opus";
            state.fileWriter.Init(name.c_str(), state.sampleRate);
            state.fileWriter.PadSilence((((uint64_t)m_curTick - m_silenceTicks) * 1000000UL) / (uint64_t)(context->fTickRate * 1000UL));
        }

        state.fileWriter.WriteSamples(m_decodeBuffer, numDecompressedSamples);

        context->logic->OnClientVoiceChat(voiceData->fromClientIndex, (float)numDecompressedSamples / (float)state.sampleRate);

        state.lastVoiceDataTick = m_curTick;
        m_lastVoiceTick = m_curTick;
    }
}
