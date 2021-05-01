
#include "net_signonstate.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool Net_SignonState_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_SignonState* data)
    {
        data->signonState = bitbuf.ReadByte();
        data->spawnCount = bitbuf.ReadLong();
        //assert(signonState >= SIGNONSTATE_NONE && signonState <= SIGNONSTATE_CHANGELEVEL);
        return !bitbuf.IsOverflowed();
    }
}
