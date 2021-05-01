
#include "svc_setview.h"
#include "base/bitfile.h"
#include "netcontants.h"

namespace NetHandlers
{
    bool SVC_SetView_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_SetView* data)
    {
        data->entIndex = bitbuf.ReadUBitLong(MAX_EDICT_BITS);
        return !bitbuf.IsOverflowed();
    }
}
