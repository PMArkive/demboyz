
#include "svc_setpause.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_SetPause_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_SetPause* data)
    {
        data->isPaused = bitbuf.ReadOneBit() != 0;
        return !bitbuf.IsOverflowed();
    }
}
