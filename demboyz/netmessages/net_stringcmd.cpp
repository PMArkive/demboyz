
#include "net_stringcmd.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool Net_StringCmd_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_StringCmd* data)
    {
        bitbuf.ReadString(data->command, sizeof(data->command));
        return !bitbuf.IsOverflowed();
    }
}
