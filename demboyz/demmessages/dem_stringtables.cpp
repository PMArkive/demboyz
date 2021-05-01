
#include "dem_stringtables.h"
#include "demofile/demofile.h"
#include "netmessages/netcontants.h"
#include "sourcesdk/bitbuf.h"

static void StringTableEntry_BitRead(bf_read& bitbuf, DemMsg::Dem_StringTables::StringTableEntry* data)
{
    using StringTableEntry = DemMsg::Dem_StringTables::StringTableEntry;

    char entryName[StringTableEntry::ENTRYNAME_MAX_LENGTH];
    bitbuf.ReadString(entryName, sizeof(entryName));

    if (bitbuf.ReadOneBit() != 0)
    {
        const int32_t numBytes = bitbuf.ReadWord();
        data->data.reset(numBytes);
        bitbuf.ReadBytes(data->data.begin(), numBytes);
    }
    else
    {
        data->data.reset(0);
    }
}

static void StringTable_BitRead(bf_read& bitbuf, DemMsg::Dem_StringTables::StringTable* data)
{
    using StringTable = DemMsg::Dem_StringTables::StringTable;
    using StringTableEntry = DemMsg::Dem_StringTables::StringTableEntry;

    char tableName[StringTable::TABLENAME_MAX_LENGTH];
    bitbuf.ReadString(tableName, sizeof(tableName));
    data->tableName.assign(tableName);

    data->entries.reset(bitbuf.ReadWord());
    for (StringTableEntry& entry : data->entries)
    {
        StringTableEntry_BitRead(bitbuf, &entry);
    }
    if (bitbuf.ReadOneBit() != 0)
    {
        data->entriesClientSide.reset(bitbuf.ReadWord());
        for (StringTableEntry& entry : data->entriesClientSide)
        {
            StringTableEntry_BitRead(bitbuf, &entry);
        }
    }
    else
    {
        data->entriesClientSide.reset(0);
    }
}

namespace DemHandlers
{
    bool Dem_StringTables_FileRead_Internal(FileRead& demofile, DemMsg::Dem_StringTables* data)
    {
        using StringTable = DemMsg::Dem_StringTables::StringTable;

        Array<uint8_t> buffer = demofile.ReadRawData(MAX_STRINGTABLE_DATA);
        bf_read bitbuf(buffer.begin(), buffer.length());

        data->stringtables.reset(bitbuf.ReadByte());
        for (StringTable& table : data->stringtables)
        {
            StringTable_BitRead(bitbuf, &table);
        }

        // copy trailing bits for binary completeness
        const unsigned int numBitsLeft = bitbuf.GetNumBitsLeft();
        assert(numBitsLeft < 8);
        data->numTrailingBits = numBitsLeft;
        if (numBitsLeft > 0)
        {
            data->trailingBitsValue = bitbuf.ReadUBitLong(numBitsLeft);
        }
        return !bitbuf.IsOverflowed();
    }
}
