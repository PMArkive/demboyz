
#include "svc_setpausetimed.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_SetPauseTimed_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_SetPauseTimed* data)
    {
        data->isPaused = bitbuf.ReadOneBit() != 0;
        data->time = bitbuf.ReadFloat();
        return !bitbuf.IsOverflowed();
    }
}
