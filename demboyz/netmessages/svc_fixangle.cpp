
#include "svc_fixangle.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool SVC_FixAngle_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_FixAngle* data)
    {
        data->relative = bitbuf.ReadOneBit() != 0;
        data->angle.x = bitbuf.ReadBitAngle(16);
        data->angle.y = bitbuf.ReadBitAngle(16);
        data->angle.z = bitbuf.ReadBitAngle(16);
        return !bitbuf.IsOverflowed();
    }
}
