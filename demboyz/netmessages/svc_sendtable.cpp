
#include "svc_sendtable.h"
#include "base/bitfile.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_SendTable_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_SendTable* data)
    {
        data->needsDecoder = bitbuf.ReadOneBit() != 0;
        data->dataLengthInBits = bitbuf.ReadShort();
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
