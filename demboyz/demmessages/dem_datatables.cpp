
#include "dem_datatables.h"
#include "demofile/demofile.h"
#include "sourcesdk/bitbuf.h"
#include <iostream>

namespace DemHandlers
{
    bool Dem_DataTables_FileRead_Internal(FileRead& demofile, DemMsg::Dem_DataTables* data)
    {
        Array<uint8_t> buffer = demofile.ReadRawData(DemMsg::Dem_DataTables::DATA_MAX_LENGTH);
        bf_read bitbuf(buffer.begin(), buffer.length());

        char strBuf[1024];
        while (bitbuf.ReadOneBit() != 0)
        {
            DemMsg::Dem_DataTables::SendTable sendTable;

            sendTable.needsDecoder = bitbuf.ReadOneBit() != 0;
            bitbuf.ReadString(strBuf, sizeof(strBuf));
            sendTable.name.assign(strBuf);

            sendTable.numProps = bitbuf.ReadUBitLong(PROPINFOBITS_NUMPROPS);

            for (int i = 0; i < sendTable.numProps; i++)
            {
                DemMsg::Dem_DataTables::SendProp prop;

                prop.type = bitbuf.ReadUBitLong(PROPINFOBITS_TYPE);

                bitbuf.ReadString(strBuf, sizeof(strBuf));
                prop.name.assign(strBuf);

                prop.flags = bitbuf.ReadUBitLong(PROPINFOBITS_FLAGS); // demoprotocol 2: 11

                if (prop.type == DPT_DataTable || prop.flags & SPROP_EXCLUDE)
                {
                    bitbuf.ReadString(strBuf, sizeof(strBuf));
                    prop.exclude.assign(strBuf);
                }
                else if (prop.type == DPT_Array)
                {
                    prop.elements = bitbuf.ReadUBitLong(PROPINFOBITS_NUMELEMENTS);
                }
                else
                {
                    prop.lowValue = bitbuf.ReadBitFloat();
                    prop.highValue = bitbuf.ReadBitFloat();
                    prop.bits = bitbuf.ReadUBitLong(PROPINFOBITS_NUMBITS);
                }

                sendTable.props.push_back(prop);
            }

            data->sendtables.push_back(sendTable);
        }

        int numClasses = bitbuf.ReadShort();
        data->classes.reset(numClasses);
        for (int i = 0; i < numClasses; i++)
        {
            int classID = bitbuf.ReadShort();
            DemMsg::Dem_DataTables::DataClass &dataClass = data->classes[classID];

            bitbuf.ReadString(strBuf, sizeof(strBuf));
            dataClass.className.assign(strBuf);

            bitbuf.ReadString(strBuf, sizeof(strBuf));
            dataClass.datatableName.assign(strBuf);
        }

        return demofile.IsOk();
    }
}
