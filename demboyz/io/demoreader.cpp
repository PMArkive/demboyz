#include "demoreader.h"
#include "demofile/demofile.h"
#include "demofile/demotypes.h"

#include "game/sourcecontext.h"
#include "netmessages/nethandlers.h"
#include "netmessages/netcontants.h"
#include "demmessages/demhandlers.h"
#include "demmessages/dem_stringtables.h"
#include "sourcesdk/bitbuf.h"
#include <cstdint>

PacketTrailingBits ParsePacket(uint8_t* packet, size_t length,
                               SourceGameContext& context,
                               const NetHandlers::NetDataStructArray& netDataStructs)
{
    assert(length <= NET_MAX_PAYLOAD);
    bf_read bitbuf(packet, length);
    NetPacket netPacket;
    while (bitbuf.GetNumBitsLeft() >= NETMSG_TYPE_BITS)
    {
        netPacket.type = bitbuf.ReadUBitLong(NETMSG_TYPE_BITS);
        netPacket.data = netDataStructs[netPacket.type];
        NetHandlers::NetMsg_BitRead(netPacket.type, bitbuf, context, netPacket.data);
        context.OnNetPacket(netPacket);
    }

    PacketTrailingBits trailingBits;
    trailingBits.numTrailingBits = bitbuf.GetNumBitsLeft();
    if (trailingBits.numTrailingBits)
    {
        trailingBits.value = bitbuf.ReadUBitLong(trailingBits.numTrailingBits);
    }
    else
    {
        trailingBits.value = 0;
    }
    return trailingBits;
}

bool DemoReader::ProcessDem(std::FILE* inputFp, SourceGameContext* context)
{
    NetHandlers::NetDataStructArray netDataStructs;
    DemHandlers::DemDataStructArray demDataStructs;
    NetHandlers::CreateNetMsgStructs(netDataStructs);
    DemHandlers::CreateDemMsgStructs(demDataStructs);

    DemoFileReader reader(inputFp);
    {
        reader.ReadDemoHeader(context->header);
        context->protocol = context->header.networkprotocol;
        context->Start();
    }

    bool dirty = false;
    try
    {
        CommandPacket packet;
        do
        {
            reader.ReadCmdHeader(packet.cmd, packet.tick);
            packet.data = demDataStructs[packet.cmd];
            DemHandlers::DemMsg_FileRead(packet.cmd, reader, packet.data);

            PacketTrailingBits trailingBits = PacketTrailingBits();
            context->StartCommandPacket(packet);

            if (packet.cmd == dem_packet || packet.cmd == dem_signon)
            {
                Array<uint8_t> buffer = reader.ReadRawData(NET_MAX_PAYLOAD);
                trailingBits = ParsePacket(buffer.begin(), buffer.length(), *context, netDataStructs);
            }

            else if (packet.cmd == dem_stringtables)
            {
                DemMsg::Dem_StringTables *stringTables = (DemMsg::Dem_StringTables *)packet.data;
                for (const auto& s : stringTables->stringtables)
                {
                    StringTable *table = context->stringTables->GetStringTable(s.tableName.c_str(), false);
                    if (!table)
                        continue;
                    for (const auto& e : s.entries)
                    {
                        StringTableEntry entry;
                        entry.string.assign(e.entryName);
                        entry.data.assign(e.data.begin(), e.data.end());
                        table->AddEntry(&entry);
                    }
                }
            }

            context->EndCommandPacket(trailingBits);
        } while (packet.cmd != dem_stop);
    }
    catch(const char *e)
    {
        dirty = true;
        fprintf(stderr, "Error: %s\n", e);
    }

    context->Finish(dirty);

    DemHandlers::DestroyDemMsgStructs(demDataStructs);
    NetHandlers::DestroyNetMsgStructs(netDataStructs);

    return !dirty;
}
