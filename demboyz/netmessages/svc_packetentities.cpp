
#include "svc_packetentities.h"
#include "base/bitfile.h"
#include "netcontants.h"
#include "netmath.h"

namespace NetHandlers
{
    bool SVC_PacketEntities_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_PacketEntities* data)
    {
        data->maxEntries = bitbuf.ReadUBitLong(MAX_EDICT_BITS);
        data->isDelta = bitbuf.ReadOneBit() != 0;
        if (data->isDelta)
        {
            data->deltaFromTick = bitbuf.ReadLong();
        }
        else
        {
            data->deltaFromTick = -1;
        }
        data->baselineIndex = bitbuf.ReadUBitLong(1);
        data->numUpdatedEntries = bitbuf.ReadUBitLong(MAX_EDICT_BITS);
        data->dataLengthInBits = bitbuf.ReadUBitLong(DELTASIZE_BITS);
        data->updateBaseline = bitbuf.ReadOneBit() != 0;
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);
/*
        int last_index = -1;
        for (int i = 0; i < data->numUpdatedEntries; i++)
        {
            last_index += 1 + bitbuf.ReadUBitVar();

            int pvs = bitbuf.ReadUBitLong(2);
            printf("%d - %d\n", last_index, pvs);
            switch(pvs)
            {
                case 0: // delta
                {

                } break;
                case 2: // enter PVS
                {
                    int iClass = bitbuf.ReadUBitLong(MAX_SERVER_CLASS_BITS);
                    int serial = bitbuf.ReadUBitLong(NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS);
                    printf("\t%d - %d\n", iClass, serial);
                } break;
                case 1: // leave PVS
                case 3: // delete
                {

                } break;
            }

        }
*/
        return !bitbuf.IsOverflowed();
    }
}
