
#include "svc_tempentities.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "netcontants.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_TempEntities_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_TempEntities* data)
    {
        data->numEntries = bitbuf.ReadUBitLong(EVENT_INDEX_BITS);
        if (context.protocol > 23)
        {
            data->dataLengthInBits = bitbuf.ReadVarInt32();
        }
        else
        {
            data->dataLengthInBits = bitbuf.ReadUBitLong(NET_MAX_PAYLOAD_BITS_OLD);
        }
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
