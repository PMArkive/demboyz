
#include "demhandlers.h"
#include "demmessages.h"

#include "dem_unknown.h"
#include "dem_packet.h"
#include "dem_synctick.h"
#include "dem_consolecmd.h"
#include "dem_usercmd.h"
#include "dem_datatables.h"
#include "dem_stop.h"
#include "dem_stringtables.h"

void DemHandlers::CreateDemMsgStructs(DemDataStructArray& demDataStructs)
{
    demDataStructs[0] = new DemMsg::Dem_Unknown();
    demDataStructs[1] = new DemMsg::Dem_Packet();
    demDataStructs[2] = new DemMsg::Dem_Packet();
    demDataStructs[3] = new DemMsg::Dem_SyncTick();
    demDataStructs[4] = new DemMsg::Dem_ConsoleCmd();
    demDataStructs[5] = new DemMsg::Dem_UserCmd();
    demDataStructs[6] = new DemMsg::Dem_DataTables();
    demDataStructs[7] = new DemMsg::Dem_Stop();
    demDataStructs[8] = new DemMsg::Dem_StringTables();
}

void DemHandlers::DestroyDemMsgStructs(DemDataStructArray& demDataStructs)
{
    delete reinterpret_cast<DemMsg::Dem_Unknown*>(demDataStructs[0]);
    delete reinterpret_cast<DemMsg::Dem_Packet*>(demDataStructs[1]);
    delete reinterpret_cast<DemMsg::Dem_Packet*>(demDataStructs[2]);
    delete reinterpret_cast<DemMsg::Dem_SyncTick*>(demDataStructs[3]);
    delete reinterpret_cast<DemMsg::Dem_ConsoleCmd*>(demDataStructs[4]);
    delete reinterpret_cast<DemMsg::Dem_UserCmd*>(demDataStructs[5]);
    delete reinterpret_cast<DemMsg::Dem_DataTables*>(demDataStructs[6]);
    delete reinterpret_cast<DemMsg::Dem_Stop*>(demDataStructs[7]);
    delete reinterpret_cast<DemMsg::Dem_StringTables*>(demDataStructs[8]);
}

#define DECLARE_DEM_HANDLER_ARRAY(funcname) \
    { \
        &DemHandlers::Dem_Unknown_##funcname, \
        &DemHandlers::Dem_Packet_##funcname, \
        &DemHandlers::Dem_Packet_##funcname, \
        &DemHandlers::Dem_SyncTick_##funcname, \
        &DemHandlers::Dem_ConsoleCmd_##funcname, \
        &DemHandlers::Dem_UserCmd_##funcname, \
        &DemHandlers::Dem_DataTables_##funcname, \
        &DemHandlers::Dem_Stop_##funcname, \
        &DemHandlers::Dem_StringTables_##funcname \
    }

typedef bool (*DemMsgFileReadFn)(DemHandlers::FileRead& demofile, void* data);

bool DemHandlers::DemMsg_FileRead(uint32_t type, FileRead& demofile, void* data)
{
    static const DemMsgFileReadFn demHandlers[] = DECLARE_DEM_HANDLER_ARRAY(FileRead);
    if (type >= (sizeof(demHandlers) / sizeof(DemMsgFileReadFn)))
    {
        return false;
    }
    return demHandlers[type](demofile, data);
}
