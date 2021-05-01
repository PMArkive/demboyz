
#include "net_tick.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool Net_Tick_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_Tick* data)
    {
        data->tick = bitbuf.ReadUBitLong(32);
        data->hostFrameTime = bitbuf.ReadUBitLong(16);
        data->hostFrameTimeStdDev = bitbuf.ReadUBitLong(16);
        return !bitbuf.IsOverflowed();
    }
}
