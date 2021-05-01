
#include "net_disconnect.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool Net_Disconnect_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_Disconnect* data)
    {
        bitbuf.ReadString(data->message, sizeof(data->message));
        return !bitbuf.IsOverflowed();
    }
}
