
#include "svc_getcvarvalue.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_GetCvarValue_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_GetCvarValue* data)
    {
        data->cookie = bitbuf.ReadSBitLong(32);
        bitbuf.ReadString(data->cvarName, sizeof(data->cvarName));
        return !bitbuf.IsOverflowed();
    }
}
