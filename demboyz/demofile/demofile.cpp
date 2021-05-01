
#include "demofile.h"
#include "demotypes.h"
#include <cassert>
#include <utility>

// DemoFileReader

size_t myfread ( void * ptr, size_t size, size_t count, FILE * stream )
{
    size_t ret = fread(ptr, size, count, stream);
    if (ret != count)
        throw("fread error");
    return ret;
}

DemoFileReader::DemoFileReader(FILE* fp):
    m_demoFp(fp)
{
}

bool DemoFileReader::IsOk() const
{
    return ferror(m_demoFp) == 0;
}

void DemoFileReader::ReadDemoHeader(demoheader_t& header)
{
    myfread(&header, sizeof(demoheader_t), 1, m_demoFp);
}

int32_t DemoFileReader::ReadRawData(uint8_t* buffer, int32_t maxLength)
{
    FILE* fp = m_demoFp;

    int32_t size;
    myfread(&size, sizeof(int32_t), 1, fp);

    if (buffer && (maxLength < size))
    {
        return -1;
    }

    if (buffer)
    {
        myfread(buffer, 1, size, fp);
    }
    else
    {
        fseek(fp, size, SEEK_CUR);
    }
    return size;
}

Array<uint8_t> DemoFileReader::ReadRawData(int32_t maxLength)
{
    FILE* fp = m_demoFp;

    int32_t size;
    myfread(&size, sizeof(int32_t), 1, fp);

    Array<uint8_t> data;
    if (maxLength < size)
    {
        return data;
    }

    data.reset(size);
    myfread(data.begin(), 1, size, fp);
    return data;
}

void DemoFileReader::ReadSequenceInfo(int32_t& seqNum1, int32_t& seqNum2)
{
    FILE* fp = m_demoFp;
    myfread(&seqNum1, sizeof(int32_t), 1, fp);
    myfread(&seqNum2, sizeof(int32_t), 1, fp);
}

void DemoFileReader::ReadCmdInfo(democmdinfo_t& info)
{
    myfread(&info, sizeof(democmdinfo_t), 1, m_demoFp);
}

void DemoFileReader::ReadCmdHeader(unsigned char& cmd, int32_t& tick)
{
    FILE* fp = m_demoFp;
    myfread(&cmd, 1, sizeof(unsigned char), fp);
    if (cmd > dem_lastcmd)
    {
        cmd = dem_stop;
    }
    if (cmd != dem_stop)
        myfread(&tick, 1, sizeof(int32_t), fp);
}

int32_t DemoFileReader::ReadUserCmd(int32_t& cmdNum, uint8_t* buffer, int32_t maxLength)
{
    myfread(&cmdNum, sizeof(int32_t), 1, m_demoFp);
    return ReadRawData(buffer, maxLength);
}

Array<uint8_t> DemoFileReader::ReadUserCmd(int32_t& cmdNum, int32_t maxLength)
{
    myfread(&cmdNum, sizeof(int32_t), 1, m_demoFp);
    return ReadRawData(maxLength);
}
