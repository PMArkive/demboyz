
#include "svc_prefetch.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "netcontants.h"

namespace NetHandlers
{
    bool SVC_Prefetch_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_Prefetch* data)
    {
        data->type = NetMsg::SVC_Prefetch::SOUND;
        if (context.protocol > 23)
        {
            data->soundIndex = bitbuf.ReadUBitLong(MAX_SOUND_INDEX_BITS);
        }
        else
        {
            data->soundIndex = bitbuf.ReadUBitLong(MAX_SOUND_INDEX_BITS_OLD);
        }
        return !bitbuf.IsOverflowed();
    }
}
