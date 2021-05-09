
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
    demDataStructs[dem_unknown] = new DemMsg::Dem_Unknown();
    demDataStructs[dem_signon] = new DemMsg::Dem_Packet();
    demDataStructs[dem_packet] = new DemMsg::Dem_Packet();
    demDataStructs[dem_synctick] = new DemMsg::Dem_SyncTick();
    demDataStructs[dem_consolecmd] = new DemMsg::Dem_ConsoleCmd();
    demDataStructs[dem_usercmd] = new DemMsg::Dem_UserCmd();
    demDataStructs[dem_datatables] = new DemMsg::Dem_DataTables();
    demDataStructs[dem_stop] = new DemMsg::Dem_Stop();
    demDataStructs[dem_stringtables] = new DemMsg::Dem_StringTables();
}

void DemHandlers::DestroyDemMsgStructs(DemDataStructArray& demDataStructs)
{
    delete reinterpret_cast<DemMsg::Dem_Unknown*>(demDataStructs[dem_unknown]);
    delete reinterpret_cast<DemMsg::Dem_Packet*>(demDataStructs[dem_signon]);
    delete reinterpret_cast<DemMsg::Dem_Packet*>(demDataStructs[dem_packet]);
    delete reinterpret_cast<DemMsg::Dem_SyncTick*>(demDataStructs[dem_synctick]);
    delete reinterpret_cast<DemMsg::Dem_ConsoleCmd*>(demDataStructs[dem_consolecmd]);
    delete reinterpret_cast<DemMsg::Dem_UserCmd*>(demDataStructs[dem_usercmd]);
    delete reinterpret_cast<DemMsg::Dem_DataTables*>(demDataStructs[dem_datatables]);
    delete reinterpret_cast<DemMsg::Dem_Stop*>(demDataStructs[dem_stop]);
    delete reinterpret_cast<DemMsg::Dem_StringTables*>(demDataStructs[dem_stringtables]);
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
