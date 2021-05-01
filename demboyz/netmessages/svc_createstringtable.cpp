
#include "svc_createstringtable.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "game/stringtables.h"
#include "netmath.h"
#include "netcontants.h"

#include "sourcesdk/common.h"
#include <vector>

namespace NetHandlers
{
    bool SVC_CreateStringTable_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_CreateStringTable* data)
    {
        if (bitbuf.PeekUBitLong(8) == ':')
        {
            data->isFileNames = true;
            bitbuf.ReadByte();
        }
        else
        {
            data->isFileNames = false;
        }
        bitbuf.ReadString(data->tableName, sizeof(data->tableName));
        data->maxEntries = bitbuf.ReadWord();

        const uint32_t numEncodeBits = math::log2(data->maxEntries);
        data->numEntries = bitbuf.ReadUBitLong(numEncodeBits + 1);

        if (context.protocol > 23)
        {
            data->dataLengthInBits = bitbuf.ReadVarInt32();
        }
        else
        {
            data->dataLengthInBits = bitbuf.ReadUBitLong(NET_MAX_PAYLOAD_BITS_OLD + 3);
        }
        const uint32_t dataLengthInBytes = math::BitsToBytes(data->dataLengthInBits);

        data->isUserDataFixedSize = bitbuf.ReadOneBit() != 0;
        if (data->isUserDataFixedSize)
        {
            data->userDataSize = bitbuf.ReadUBitLong(12);
            data->userDataSizeBits = bitbuf.ReadUBitLong(4);
        }
        else
        {
            data->userDataSize = 0;
            data->userDataSizeBits = 0;
        }
        if (context.protocol > 14)
        {
            data->compressedData = bitbuf.ReadOneBit() != 0;
        }
        else
        {
            data->compressedData = false;
        }

        data->data.reset(new uint8_t[dataLengthInBytes]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);

        StringTable *table = context.stringTables->GetStringTable(data->tableName, true);
        table->maxEntries = data->maxEntries;
        table->isUserDataFixedSize = data->isUserDataFixedSize;
        table->userDataSize = data->userDataSize;
        table->userDataSizeBits = data->userDataSizeBits;
        table->entryBits = math::log2(table->maxEntries);

        if (data->compressedData)
        {
            bf_read bitbuf2(data->data.get(), dataLengthInBytes, data->dataLengthInBits);
            const uint32_t decompressedNumBytes = bitbuf2.ReadUBitLong(32);
            const uint32_t compressedNumBytes = bitbuf2.ReadUBitLong(32);
            std::unique_ptr<uint8_t[]> compressedData(new uint8[compressedNumBytes]);
            std::unique_ptr<uint8_t[]> uncompressedData(new uint8[decompressedNumBytes]);
            bitbuf2.ReadBytes(compressedData.get(), compressedNumBytes);

            uint32_t numWritten = COM_BufferToBufferDecompress(uncompressedData.get(), decompressedNumBytes, compressedData.get(), compressedNumBytes);
            assert(numWritten == decompressedNumBytes);
            bitbuf2 = bf_read(uncompressedData.get(), decompressedNumBytes);
            table->ParseUpdate(bitbuf2, data->numEntries, context);
        }
        else if(dataLengthInBytes)
        {
            bf_read bitbuf2(data->data.get(), dataLengthInBytes, data->dataLengthInBits);
            table->ParseUpdate(bitbuf2, data->numEntries, context);
        }

        return !bitbuf.IsOverflowed();
    }
}
