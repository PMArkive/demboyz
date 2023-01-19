#pragma once

#include "opusfilewriter.h"
#include "game/sourcecontext.h"
#include <SKP_Silk_control.h>

struct CELTMode;

enum eCodec
{
    CODEC_NONE = 0,
    CODEC_CELT = 1,
    CODEC_SPEEX = 2,
    CODEC_STEAM = 2
};

class CeltVoiceDecoder
{
public:
    bool DoInit(CELTMode* celtMode, uint32_t frameSizeSamples, uint32_t encodedFrameSizeBytes);
    void Destroy();
    void Reset();

    int Decompress(const uint8_t* compressedData, uint32_t compressedBytes, int16_t* uncompressedData, uint32_t maxUncompressedSamples);
    void DecodeFrame(const uint8_t* compressedData, int16_t* uncompressedData);

private:
    static const int sCeltChannels = 1;

private:
    struct CELTDecoder* m_celtDecoder = NULL;
    uint32_t m_frameSizeSamples = 0;
    uint32_t m_encodedFrameSizeBytes = 0;
};

class SilkVoiceDecoder
{
public:
    bool DoInit(int32_t sampleRate);
    void Destroy();
    void Reset();

    int Decompress(const uint8_t* compressedData, uint32_t compressedBytes, int16_t* uncompressedData, uint32_t maxUncompressedSamples);

private:
    static const int sSilkChannels = 1;

private:
    void* m_Silk_DecoderState = NULL;
    SKP_SILK_SDK_DecControlStruct m_Silk_DecoderControl;
};

class VoiceDataWriter
{
public:
    VoiceDataWriter(SourceGameContext *context, const char* outputPath, bool bSkipSilence);
    bool init();

    void Start();
    void Finish();
    void End();

    void StartCommandPacket(const CommandPacket& packet);
    void EndCommandPacket(const PacketTrailingBits& trailingBits);

	void OnNetPacket(NetPacket& packet);

private:
    struct PlayerVoiceState
    {
        CeltVoiceDecoder celt_decoder;
        SilkVoiceDecoder silk_decoder;

        OpusFileWriter fileWriter;
        int32_t lastVoiceDataTick = -1;
        int sampleRate = 0;
    };

    int ParseSteamVoicePacket(const uint8_t* bytes, int numBytes, PlayerVoiceState& state);

private:
    SourceGameContext *context = nullptr;
    CELTMode* m_celtMode = nullptr;
    std::map<std::string, PlayerVoiceState> m_playerVoiceStates;

    int32_t m_curTick = 0;
    int32_t m_lastTick = 0;

    int32_t m_lastVoiceTick = 0;
    int32_t m_silenceTicks = 0;
    int32_t m_silenceTicksStart = 0;
    const char* m_outputPath = nullptr;
    bool m_bSkipSilence = true;

    int16_t m_decodeBuffer[32768];

    static const int sQuality = 3;
    eCodec m_Codec = CODEC_NONE;

public:
    int32_t m_tickBase = 0;
    bool m_isSilenced = false;
    std::vector<std::pair<int32_t, int32_t>> m_silence;
};
