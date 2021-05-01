
#include "net_setconvar.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    using cvar_t = NetMsg::Net_SetConVar::cvar_t;

    bool Net_SetConVar_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_SetConVar* data)
    {
        data->cvars.resize(bitbuf.ReadByte());
        for (cvar_t& cvar : data->cvars)
        {
            bitbuf.ReadString(cvar.name, sizeof(cvar.name));
            bitbuf.ReadString(cvar.value, sizeof(cvar.value));
            //printf("%s -> %s\n", cvar.name, cvar.value);
        }
        return !bitbuf.IsOverflowed();
    }
}
