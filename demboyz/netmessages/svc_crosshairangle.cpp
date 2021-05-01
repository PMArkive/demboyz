
#include "svc_crosshairangle.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_CrosshairAngle_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_CrosshairAngle* data)
    {
        data->angle.x = bitbuf.ReadBitAngle(16);
        data->angle.y = bitbuf.ReadBitAngle(16);
        data->angle.z = bitbuf.ReadBitAngle(16);
        return !bitbuf.IsOverflowed();
    }
}
