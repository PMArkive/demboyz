
#include "svc_usermessage.h"
#include "base/bitfile.h"
#include "netmath.h"
#include "netcontants.h"
#include <cassert>

namespace NetHandlers
{
    bool SVC_UserMessage_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_UserMessage* data)
    {
        data->msgType = bitbuf.ReadByte();
        data->dataLengthInBits = bitbuf.ReadUBitLong(11);
        assert(math::BitsToBytes(data->dataLengthInBits) <= MAX_USER_MSG_DATA);
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
        return !bitbuf.IsOverflowed();
    }
}
