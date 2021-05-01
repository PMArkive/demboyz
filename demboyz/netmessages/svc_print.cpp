
#include "svc_print.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_Print_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_Print* data)
    {
        bitbuf.ReadString(data->text, sizeof(data->text));
        return !bitbuf.IsOverflowed();
    }
}
