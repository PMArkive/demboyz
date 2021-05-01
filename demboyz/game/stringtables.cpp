
#include "base/bitfile.h"
#include "stringtables.h"
#include "game/sourcecontext.h"
#include <iostream>

static size_t strlcpy(char * dst, const char * src, size_t maxlen) {
    const size_t srclen = strlen(src);
    if (srclen + 1 < maxlen) {
        memcpy(dst, src, srclen + 1);
    } else if (maxlen != 0) {
        memcpy(dst, src, maxlen - 1);
        dst[maxlen-1] = '\0';
    }
    return srclen;
}

StringTableContainer::StringTableContainer(SourceGameContext *context):
    context(context)
{
}

StringTable *StringTableContainer::GetStringTable(const char *name, bool create)
{
    StringTable *table = nullptr;
    for (auto& t : tables)
    {
        if (t.tableName.compare(name) == 0)
        {
            table = &t;
            break;
        }
    }
    if (!table && create)
    {
        StringTable ttable;
        tables.emplace_back(ttable);
        table = &tables.back();
        table->id = tables.size() - 1;
        table->tableName.assign(name);
        table->entries.clear();
        context->OnStringtable(table);
    }
    return table;
}

void StringTableContainer::DumpAll()
{
    for (auto& t : tables)
    {
        std::cout << "[STRINGTABLE] " << t.tableName << " (" << t.entries.size() << ")\n";
        int idx = 0;
        for (auto& e : t.entries)
        {
            std::cout << idx++ << ":\t" << e.string << " (" << e.data << ")\n";
        }
        std::cout << "\n";
    }
}

StringTableEntry *StringTable::FindEntry(const char *string)
{
    for (auto& e : entries)
    {
        if (e.string.compare(string) == 0)
            return &e;
    }
    return nullptr;
}

#define SUBSTRING_BITS 5
struct StringHistoryEntry
{
    char string[(1 << SUBSTRING_BITS)];
};

void StringTable::AddEntry(StringTableEntry *entry)
{
    StringTableEntry *e = FindEntry(entry->string.c_str());
    if (!e)
    {
        entries.emplace_back(*entry);
        return;
    }

    e->string.assign(entry->string);
    e->data.assign(entry->data);
}

void StringTable::ParseUpdate(bf_read& bitbuf, int numEntries, SourceGameContext& context)
{
    std::vector<StringHistoryEntry> history;
    int entryIndex = -1;
    for (int i = 0; i < numEntries; ++i)
    {
        entryIndex++;

        if (bitbuf.ReadOneBit() == 0)
        {
            entryIndex = bitbuf.ReadUBitLong(entryBits);
        }

        const char *pEntry = NULL;
        char entry[1024+1];
        char substr[1024];
        if (bitbuf.ReadOneBit() != 0)
        {
            bool substringcheck = bitbuf.ReadOneBit() != 0;
            if (substringcheck)
            {
                int index = bitbuf.ReadUBitLong(5);
                int bytestocopy = bitbuf.ReadUBitLong(SUBSTRING_BITS);
                strlcpy(entry, history.at(index).string, MIN((int)sizeof(StringHistoryEntry::string), bytestocopy + 1));
                bitbuf.ReadString(substr, sizeof(substr));
                strncat(entry, substr, sizeof(entry) - 1);
            }
            else
            {
                bitbuf.ReadString(entry, sizeof(entry));
            }
            pEntry = entry;
        }

        const int MAX_USERDATA_BITS = 14;
        unsigned char tempbuf[(1 << MAX_USERDATA_BITS)] = { 0 };
        const void *pUserData = NULL;
        int nUserDataBytes = 0;
        if (bitbuf.ReadOneBit() != 0)
        {
            if (isUserDataFixedSize)
            {
                bitbuf.ReadBits(tempbuf, userDataSizeBits);
            }
            else
            {
                nUserDataBytes = bitbuf.ReadUBitLong(MAX_USERDATA_BITS);
                bitbuf.ReadBytes(tempbuf, nUserDataBytes);
            }
            pUserData = tempbuf;
        }

        if (pEntry == NULL)
        {
            pEntry = "";
        }

        if (entryIndex >= (int)entries.size())
        {
            StringTableEntry entry = {};
            entry.string.assign(pEntry);
            entries.emplace_back(entry);
}
        else
        {
            pEntry = entries[entryIndex].string.c_str();
        }

        assert(entryIndex < (int)entries.size());

        entries[entryIndex].data.assign((const char *)pUserData, nUserDataBytes);

        if (callback)
            callback(id, entryIndex);

        if (history.size() > 31)
        {
            history.erase(history.begin());
        }

        StringHistoryEntry she;
        strlcpy(she.string, pEntry, sizeof(she.string));
        history.emplace_back(she);
    }
}