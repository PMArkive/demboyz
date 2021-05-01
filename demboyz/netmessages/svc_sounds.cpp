
#include "svc_sounds.h"
#include "base/bitfile.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_Sounds_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_Sounds* data)
    {
        data->reliableSound = bitbuf.ReadOneBit() != 0;
        if (data->reliableSound)
        {
            data->numSounds = 1;
            data->dataLengthInBits = bitbuf.ReadUBitLong(8);
        }
        else
        {
            data->numSounds = bitbuf.ReadUBitLong(8);
            data->dataLengthInBits = bitbuf.ReadUBitLong(16);
        }
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
