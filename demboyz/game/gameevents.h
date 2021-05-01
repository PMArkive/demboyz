
#pragma once

#include "netmessages/netcontants.h"
#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

class bf_read;

namespace GameEvents
{
    enum EventValueType : uint8_t
    {
        Local = 0,      // not networked
        String = 1,     // zero terminated ASCII string
        Float = 2,      // 32 bit float
        Long = 3,       // 32 bit signed int
        Short = 4,      // 16 bit signed int
        Byte = 5,       // 8 bit unsigned int
        Bool = 6        // 1 bit unsigned int
    };

    struct EventValue
    {
        EventValueType type;
        char name[MAX_EVENT_NAME_LENGTH];
    };

    struct EventDescriptor
    {
        uint16_t id;
        char name[MAX_EVENT_NAME_LENGTH];
        std::vector<EventValue> values;
    };

    struct EventData
    {
        EventValueType type;
        union
        {
            float       flValue;
            int32_t     i32Value;
            int16_t     i16Value;
            uint8_t     u8Value;
            bool        bValue;
        };
        std::string strValue;
    };

    using EventDataMap = std::map<std::string, EventData>;
    EventDataMap ParseEventData(bf_read& bitbuf, const EventDescriptor& desc);
    void PrintEvent(const char* name, EventDataMap& data);
}
