
#include "svc_bspdecal.h"
#include "base/bitfile.h"
#include "netcontants.h"

namespace NetHandlers
{
    bool SVC_BSPDecal_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_BSPDecal* data)
    {
        bitbuf.ReadBitVec3Coord(data->position);
        data->decalTextureIndex = bitbuf.ReadUBitLong(MAX_DECAL_INDEX_BITS);
        if (bitbuf.ReadOneBit() != 0)
        {
            data->entIndex = bitbuf.ReadUBitLong(MAX_EDICT_BITS);
            data->modelIndex = bitbuf.ReadUBitLong(SP_MODEL_INDEX_BITS);
        }
        else
        {
            data->entIndex = 0;
            data->modelIndex = 0;
        }
        data->lowPriority = bitbuf.ReadOneBit() != 0;
        return !bitbuf.IsOverflowed();
    }
}
