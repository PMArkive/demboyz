
#pragma once

#include <cstdint>
#include <sstream>
#include <array>
#include "netmessages.h"

class bf_read;

namespace NetHandlers
{
    using BitRead = bf_read;
}

struct SourceGameContext;

#if !defined(MAX_OSPATH)
#define MAX_OSPATH 260 // max length of a filesystem pathname
#endif

#define DECLARE_NET_HANDLERS(msgname) \
    namespace NetHandlers \
    { \
        bool msgname##_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::msgname* data); \
        inline bool msgname##_BitRead(BitRead& bitbuf, SourceGameContext& context, void* data) \
        { \
            return msgname##_BitRead_Internal(bitbuf, context, reinterpret_cast<NetMsg::msgname*>(data)); \
        } \
    }

namespace NetHandlers
{
    using NetDataStructArray = std::array<void*, NetMsg::SVC_LASTMSG + 1>;
    void CreateNetMsgStructs(NetDataStructArray& netDataStructs);
    void DestroyNetMsgStructs(NetDataStructArray& netDataStructs);

    bool NetMsg_BitRead(uint32_t type, BitRead& bitbuf, SourceGameContext& context, void* data);
}
