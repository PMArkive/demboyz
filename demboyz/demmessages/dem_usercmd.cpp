
#include "dem_usercmd.h"
#include "demofile/demofile.h"

namespace DemHandlers
{
    bool Dem_UserCmd_FileRead_Internal(FileRead& demofile, DemMsg::Dem_UserCmd* data)
    {
        data->commandData = demofile.ReadUserCmd(data->commandNum, DemMsg::Dem_UserCmd::COMMANDDATA_MAX_LENGTH);
        return demofile.IsOk();
    }
}
