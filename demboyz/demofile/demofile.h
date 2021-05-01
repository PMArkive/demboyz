
#pragma once

#include <cstdint>
#include <cstdio>
#include "base/array.h"

struct demoheader_t;
struct democmdinfo_t;

class DemoFileReader
{
public:
    DemoFileReader(FILE* fp);

    bool IsOk() const;
    void ReadDemoHeader(demoheader_t& header);
    int32_t ReadRawData(uint8_t* buffer, int32_t maxLength);
    Array<uint8_t> ReadRawData(int32_t maxLength);
    void ReadSequenceInfo(int32_t& seqNum1, int32_t& seqNum2);
    void ReadCmdInfo(democmdinfo_t& info);
    void ReadCmdHeader(unsigned char& cmd, int32_t& tick);
    int32_t ReadUserCmd(int32_t& cmdNum, uint8_t* buffer, int32_t maxLength);
    Array<uint8_t> ReadUserCmd(int32_t& cmdNum, int32_t maxLength);

private:
    FILE* m_demoFp;
};
