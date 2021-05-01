
#include "svc_gameeventlist.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "netcontants.h"
#include "netmath.h"

using EventDescriptor = GameEvents::EventDescriptor;
using EventValue = GameEvents::EventValue;

uint32_t CalculateNumDataBits(const std::vector<EventDescriptor>& eventDescriptors)
{
    uint32_t numBits = 0;
    for (const EventDescriptor& event : eventDescriptors)
    {
        numBits += MAX_EVENT_BITS;
        // +1 for null char
        numBits += (8 * (1 + strlen(event.name)));
        // +1 for null bits
        numBits += (3 * (1 + event.values.size()));
        for (const EventValue& value : event.values)
        {
            // +1 for null char
            numBits += (8 * (1 + strlen(value.name)));
        }
    }
    return numBits;
}

namespace NetHandlers
{
    bool SVC_GameEventList_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_GameEventList* data)
    {
        data->eventDescriptors.resize(bitbuf.ReadUBitLong(MAX_EVENT_BITS));
        data->dataLengthInBits = bitbuf.ReadUBitLong(20);
        for (EventDescriptor& event : data->eventDescriptors)
        {
            event.id = bitbuf.ReadUBitLong(MAX_EVENT_BITS);
            bitbuf.ReadString(event.name, sizeof(event.name));
            EventValue value;
            while ((value.type = static_cast<GameEvents::EventValueType>(bitbuf.ReadUBitLong(3))) > 0)
            {
                bitbuf.ReadString(value.name, sizeof(value.name));
                event.values.push_back(value);
            }
            event.values.shrink_to_fit();
        }

        if (!context.gameEventList)
        {
            context.gameEventList = new NetMsg::SVC_GameEventList(*data);
        }

        return !bitbuf.IsOverflowed();
    }
}
