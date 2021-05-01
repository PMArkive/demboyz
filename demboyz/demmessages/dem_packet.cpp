
#include "dem_packet.h"
#include "demofile/demofile.h"
#include "netmessages/nethandlers.h"

namespace DemHandlers
{
    bool Dem_Packet_FileRead_Internal(FileRead& demofile, DemMsg::Dem_Packet* data)
    {
        demofile.ReadCmdInfo(data->cmdInfo);
        demofile.ReadSequenceInfo(data->sequenceNum1, data->sequenceNum2);
        return demofile.IsOk();
    }
}
