
#include "gameevents.h"
#include "base/bitfile.h"
#include <stdio.h>
#include <iostream>

namespace GameEvents
{
    EventDataMap ParseEventData(bf_read& bitbuf, const EventDescriptor& desc)
    {
        EventDataMap data;
        char tempStr[MAX_EVENT_BYTES];
        for (const EventValue& value : desc.values)
        {
            EventData& eventData = data[value.name];
            eventData.type = value.type;
            switch(value.type)
            {
            case EventValueType::String:
            {
                const bool ok = bitbuf.ReadString(tempStr, sizeof(tempStr), false);
                assert(ok);
                eventData.strValue.assign(tempStr);
                break;
            }
            case EventValueType::Float:
            {
                eventData.flValue = bitbuf.ReadFloat();
                break;
            }
            case EventValueType::Long:
            {
                eventData.i32Value = bitbuf.ReadSBitLong(32);
                break;
            }
            case EventValueType::Short:
            {
                eventData.i16Value = bitbuf.ReadSBitLong(16);
                break;
            }
            case EventValueType::Byte:
            {
                eventData.u8Value = bitbuf.ReadUBitLong(8);
                break;
            }
            case EventValueType::Bool:
            {
                eventData.bValue = bitbuf.ReadOneBit() != 0;
                break;
            }
            case EventValueType::Local:
            default:
                assert(false);
                break;
            }
        }
        return data;
    }

    void PrintEvent(const char* name, EventDataMap& data)
    {
        std::cout << "[EVENT] " << name << "\n";
        for (const auto& d : data)
        {
            std::cout << "\t" << d.first << ": ";
            switch(d.second.type)
            {
            case GameEvents::EventValueType::String:
            {
                std::cout << d.second.strValue << "\n";
            } break;
            case GameEvents::EventValueType::Float:
            {
                std::cout << d.second.flValue << "\n";
            } break;
            case GameEvents::EventValueType::Long:
            {
                std::cout << d.second.i32Value << "\n";
            } break;
            case GameEvents::EventValueType::Short:
            {
                std::cout << d.second.i16Value << "\n";
            } break;
            case GameEvents::EventValueType::Byte:
            {
                std::cout << d.second.u8Value << "\n";
            } break;
            case GameEvents::EventValueType::Bool:
            {
                std::cout << d.second.bValue << "\n";
            } break;
            default:
            {
            } break;
            }
        }
    }
}
