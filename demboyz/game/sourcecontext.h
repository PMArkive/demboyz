
#pragma once

#include "stringtables.h"
#include "game/gameevents.h"
#include "demofile/demotypes.h"
#include <cstdint>

namespace NetMsg
{
    struct SVC_GameEventList;
}
struct Logic;
class VoiceDataWriter;

struct CommandPacket
{
    unsigned char cmd;
    int32_t tick;

    void* data;
};

struct NetPacket
{
    int32_t type;
    void* data;
};

struct PacketTrailingBits
{
    uint32_t numTrailingBits;
    uint32_t value;
};

#define MAX_PLAYERS 65
#define MAX_PLAYER_NAME_LENGTH 32
#define SIGNED_GUID_LEN 32
#define MAX_CUSTOM_FILES 4

// Engine player info, no game related infos here
// If you change this, change the two byteswap defintions:
// cdll_client_int.cpp and cdll_engine_int.cpp
typedef struct player_info_s
{
	// scoreboard information
	char			name[MAX_PLAYER_NAME_LENGTH];
	// local server user ID, unique while server is running
	int				userID;
	// global unique player identifer
	char			guid[SIGNED_GUID_LEN + 1];
	// friends identification number
	uint32_t		friendsID;
	// friends name
	char			friendsName[MAX_PLAYER_NAME_LENGTH];
	// true, if player is a bot controlled by game.dll
	bool			fakeplayer;
	// true if player is the HLTV proxy
	bool			ishltv;
#if defined( REPLAY_ENABLED )
	// true if player is the Replay proxy
	bool			isreplay;
#endif
	// custom files CRC for this player
	uint32_t		customFiles[MAX_CUSTOM_FILES];
	// this counter increases each time the server downloaded a new file
	unsigned char	filesDownloaded;
} player_info_t;

struct SourceGameContext
{
    SourceGameContext(std::string outputDir, std::string outputDirVoice, bool bSkipSilence);
    ~SourceGameContext();
    bool init();

    void Start();
    void Finish(bool dirty);
    void End();

    void StartCommandPacket(const CommandPacket& packet);
    void EndCommandPacket(const PacketTrailingBits& trailingBits);
	bool IgnoreNetPacketType(int32_t type);

	void OnNetPacket(NetPacket& packet);
	void OnGameEvent(const char *name, GameEvents::EventDataMap &data);
	void OnStringtable(StringTable* table);
	void UserInfoChanged(int tableIdx, int entryIdx);

    std::string outputDir;
	std::string outputDirVoice;
	bool m_bSkipSilence;

    FILE* outputFp;
	Logic* logic;

	demoheader_t header;
    int16_t protocol;
    NetMsg::SVC_GameEventList* gameEventList = nullptr;
    StringTableContainer* stringTables = nullptr;
	VoiceDataWriter* voiceWriter = nullptr;

	int32_t curTick = -1;
	int32_t curFrame = -1;

	float fTickInterval = -1.f;
	float fTickRate = -1.f;

	struct
	{
		bool connected = false;
		player_info_t info;
	} players[MAX_PLAYERS];

	uint8_t *userIdLookUp = nullptr;
};
