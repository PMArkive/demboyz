
#pragma once

#include <cstdint>
#include <array>
#include "demmessages.h"

class DemoFileReader;
namespace DemHandlers
{
    using FileRead = DemoFileReader;
}

#define DECLARE_DEM_HANDLERS(msgname) \
    namespace DemHandlers \
    { \
        bool msgname##_FileRead_Internal(FileRead& demofile, DemMsg::msgname* data); \
        inline bool msgname##_FileRead(FileRead& demofile, void* data) \
        { \
            return msgname##_FileRead_Internal(demofile, reinterpret_cast<DemMsg::msgname*>(data)); \
        } \
    }

namespace DemHandlers
{
    using DemDataStructArray = std::array<void*, dem_lastcmd + 1>;
    void CreateDemMsgStructs(DemDataStructArray& demDataStructs);
    void DestroyDemMsgStructs(DemDataStructArray& demDataStructs);

    bool DemMsg_FileRead(uint32_t type, FileRead& demofile, void* data);
}
