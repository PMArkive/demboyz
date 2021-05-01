
#include "svc_voicedata.h"
#include "svc_voiceinit.h"
#include "base/bitfile.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_VoiceData_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_VoiceData* data)
    {
        data->fromClientIndex = bitbuf.ReadByte();
        data->proximity = !!bitbuf.ReadByte();
        data->dataLengthInBits = bitbuf.ReadWord();
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
