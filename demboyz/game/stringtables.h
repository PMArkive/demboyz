
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

class bf_read;
class SourceGameContext;

struct StringTableEntry
{
    std::string string;
    std::string data;
};

struct StringTable
{
    int id;
    std::string tableName;
    uint16_t maxEntries;
    bool isUserDataFixedSize;
    uint16_t userDataSize;
    uint8_t userDataSizeBits;
    int entryBits;

    void ParseUpdate(bf_read& bitbuf, int numEntries, SourceGameContext& context);

    void AddEntry(StringTableEntry *entry);
    StringTableEntry *FindEntry(const char *string);

    std::vector<StringTableEntry> entries;
    std::function<void(int, int)> callback;
};

struct StringTableContainer
{
    StringTableContainer(SourceGameContext *context);

    StringTable *GetStringTable(const char *name, bool create);

    void DumpAll();

    SourceGameContext *context;
    std::vector<StringTable> tables;
};

