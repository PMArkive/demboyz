
#include "svc_menu.h"
#include "base/bitfile.h"

using DialogType = NetMsg::SVC_Menu::DialogType;

namespace NetHandlers
{
    bool SVC_Menu_BitRead_Internal(BitRead& bitbuf, SourceGameContext& context, NetMsg::SVC_Menu* data)
    {
        data->type = static_cast<DialogType>(bitbuf.ReadShort());
        data->dataLengthInBytes = bitbuf.ReadWord();
        data->menuBinaryKeyValues.reset(new uint8_t[data->dataLengthInBytes]);
        bitbuf.ReadBytes(data->menuBinaryKeyValues.get(), data->dataLengthInBytes);
        return !bitbuf.IsOverflowed();
    }
}
