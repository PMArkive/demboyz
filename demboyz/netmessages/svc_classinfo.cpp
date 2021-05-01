
#include "svc_classinfo.h"
#include "base/bitfile.h"
#include "netmath.h"

using class_t = NetMsg::SVC_ClassInfo::class_t;

namespace NetHandlers
{
    bool SVC_ClassInfo_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_ClassInfo* data)
    {
        const int16_t numServerClasses = bitbuf.ReadShort();
        const bool createOnClient = bitbuf.ReadOneBit() != 0;
        data->numServerClasses = numServerClasses;
        data->createOnClient = createOnClient;
        if (!createOnClient)
        {
            const int numServerClassBits = math::log2(numServerClasses) + 1;
            data->serverClasses.resize(numServerClasses);
            for (class_t& serverClass : data->serverClasses)
            {
                serverClass.classID = bitbuf.ReadUBitLong(numServerClassBits);
                bitbuf.ReadString(serverClass.className, sizeof(serverClass.className));
                bitbuf.ReadString(serverClass.dataTableName, sizeof(serverClass.dataTableName));
            }
        }
        return !bitbuf.IsOverflowed();
    }
}
