
#include "dem_consolecmd.h"
#include "demofile/demofile.h"

namespace DemHandlers
{
    bool Dem_ConsoleCmd_FileRead_Internal(FileRead& demofile, DemMsg::Dem_ConsoleCmd* data)
    {
        char command[DemMsg::Dem_ConsoleCmd::COMMAND_MAX_LENGTH];
        demofile.ReadRawData(reinterpret_cast<uint8_t*>(command), sizeof(command));
        data->command.assign(command);
        return demofile.IsOk();
    }
}
