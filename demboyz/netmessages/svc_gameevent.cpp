
#include "svc_gameevent.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "netcontants.h"
#include "netmath.h"
#include "svc_gameeventlist.h"

namespace NetHandlers
{
    bool SVC_GameEvent_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_GameEvent* data)
    {
        const unsigned int numBits = bitbuf.ReadUBitLong(11);
        const size_t numBytes = math::BitsToBytes(numBits);

        data->dataLengthInBits = numBits;
        data->data.reset(new uint8_t[numBytes]);
        bitbuf.ReadBits(data->data.get(), numBits);

        BitRead bitbuf2(data->data.get(), numBytes, numBits);
        const size_t id = bitbuf2.ReadUBitLong(9);
        GameEvents::EventDataMap eventData = GameEvents::ParseEventData(bitbuf2, context.gameEventList->eventDescriptors[id]);

        context.OnGameEvent(context.gameEventList->eventDescriptors[id].name, eventData);

        return !bitbuf.IsOverflowed();
    }
}
