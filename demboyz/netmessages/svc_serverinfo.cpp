
#include "svc_serverinfo.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"

namespace NetHandlers
{
    bool SVC_ServerInfo_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_ServerInfo* data)
    {
        data->protocol = bitbuf.ReadShort();
        data->serverCount = bitbuf.ReadLong();
        data->isHLTV = bitbuf.ReadOneBit() != 0;
        data->isDedicated = bitbuf.ReadOneBit() != 0;
        data->clientCRC = bitbuf.ReadLong();
        data->maxClasses = bitbuf.ReadWord();
        if (context.protocol <= 17)
        {
            data->mapCRC = bitbuf.ReadLong();
        }
        else
        {
            bitbuf.ReadBytes(data->mapMD5, sizeof(data->mapMD5));
        }
        data->playerSlot = bitbuf.ReadByte();
        data->maxClients = bitbuf.ReadByte();
        data->tickInterval = bitbuf.ReadFloat();
        data->os = bitbuf.ReadChar();
        bitbuf.ReadString(data->gameDir, sizeof(data->gameDir));
        bitbuf.ReadString(data->mapName, sizeof(data->mapName));
        bitbuf.ReadString(data->skyName, sizeof(data->skyName));
        bitbuf.ReadString(data->hostName, sizeof(data->hostName));
        if (context.protocol > 15)
        {
            data->isReplay = bitbuf.ReadOneBit() != 0;
        }
        return !bitbuf.IsOverflowed();
    }
}
