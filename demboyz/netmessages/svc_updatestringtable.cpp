
#include "svc_updatestringtable.h"
#include "base/bitfile.h"
#include "game/sourcecontext.h"
#include "game/stringtables.h"
#include "netmath.h"
#include "netcontants.h"

namespace NetHandlers
{
    bool SVC_UpdateStringTable_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_UpdateStringTable* data)
    {
        data->tableID = bitbuf.ReadUBitLong(math::log2(MAX_TABLES));
        data->numChangedEntries = (bitbuf.ReadOneBit() != 0) ? bitbuf.ReadWord() : 1;
        data->dataLengthInBits = bitbuf.ReadUBitLong(20);
        data->data.reset(new uint8_t[math::BitsToBytes(data->dataLengthInBits)]);
        bitbuf.ReadBits(data->data.get(), data->dataLengthInBits);

        StringTable *table = &context.stringTables->tables[data->tableID];
        bf_read bitbuf2(data->data.get(), math::BitsToBytes(data->dataLengthInBits), data->dataLengthInBits);
        table->ParseUpdate(bitbuf2, data->numChangedEntries, context);

        return !bitbuf.IsOverflowed();
    }
}
