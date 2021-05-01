
#include "svc_voiceinit.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_VoiceInit_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_VoiceInit* data)
    {
        bitbuf.ReadString(data->voiceCodec, sizeof(data->voiceCodec));
        data->quality = bitbuf.ReadByte();
        if(data->quality == NetMsg::SVC_VoiceInit::QUALITY_HAS_SAMPLE_RATE)
        {
            data->sampleRate = bitbuf.ReadShort();
        }
        else
        {
            // V_strnicmp < 1 is from them, not me.
            data->sampleRate = V_strnicmp(data->voiceCodec, "vaudio_celt", sizeof(data->voiceCodec)) < 1 ? 22050 : 11025;
        }
        return !bitbuf.IsOverflowed();
    }
}
