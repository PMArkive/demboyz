
#include "net_file.h"
#include "base/bitfile.h"

namespace NetHandlers
{
    bool Net_File_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::Net_File* data)
    {
        data->transferID = bitbuf.ReadUBitLong(32);
        bitbuf.ReadString(data->filename, sizeof(data->filename));
        data->isRequest = bitbuf.ReadOneBit() != 0;
        return !bitbuf.IsOverflowed();
    }
}
