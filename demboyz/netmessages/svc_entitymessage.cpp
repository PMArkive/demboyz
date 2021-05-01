
#include "svc_entitymessage.h"
#include "base/bitfile.h"
#include "netcontants.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_EntityMessage_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_EntityMessage* data)
    {
        data->entIndex = bitbuf.ReadUBitLong(MAX_EDICT_BITS);
        data->classID = bitbuf.ReadUBitLong(MAX_SERVER_CLASS_BITS);
        data->dataLengthInBits = bitbuf.ReadUBitLong(11);
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
