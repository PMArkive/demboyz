
#include "sourcecontext.h"
#include "netmessages/netmath.h"
#include "netmessages/svc_gameeventlist.h"
#include "netmessages/usermessages.h"
#include "netmessages/svc_usermessage.h"
#include "netmessages/svc_serverinfo.h"
#include "sourcesdk/bitbuf.h"
#include "game/gameevents.h"
#include "game/logic.h"
#include "io/voicewriter/voicedatawriter.h"
#include <iostream>
#include <limits.h>

#include "netmessages/svc_voiceinit.h"
#include "netmessages/svc_voicedata.h"

SourceGameContext::SourceGameContext(std::string outputDir, std::string outputDirVoice):
    outputDir(outputDir),
    outputDirVoice(outputDirVoice)
{
    stringTables = new StringTableContainer(this);
    userIdLookUp = new uint8_t[USHRT_MAX+1];
    memset(userIdLookUp, 0xFF, USHRT_MAX+1);
}

SourceGameContext::~SourceGameContext()
{
    delete voiceWriter;
    voiceWriter = nullptr;
    delete logic;
    logic = nullptr;

    protocol = 0;
    delete gameEventList;
    gameEventList = nullptr;
    delete stringTables;
    stringTables = nullptr;
    delete userIdLookUp;
    userIdLookUp = nullptr;

    fclose(outputFp);
}

bool SourceGameContext::init()
{
    outputFp = fopen((outputDir + "/out.json").c_str(), "wb");
    if (!outputFp)
    {
        fprintf(stderr, "Error: Could not open out.json\n");
        return false;
    }

    voiceWriter = new VoiceDataWriter(this, outputDirVoice.c_str());
    logic = new Logic(this);
    return true;
}

void SourceGameContext::Start()
{
    logic->Start();
    voiceWriter->Start();
}

void SourceGameContext::Finish(bool dirty)
{
    voiceWriter->Finish();
    logic->Finish(dirty);
}

void SourceGameContext::StartCommandPacket(const CommandPacket& packet)
{
    curFrame += (packet.cmd == dem_packet);

    if(curTick == -1 && packet.tick > 0)
        return;

    curTick = packet.tick;
    logic->curTick = curTick;

    voiceWriter->StartCommandPacket(packet);
}

void SourceGameContext::EndCommandPacket(const PacketTrailingBits& trailingBits)
{
    if(curTick == -1)
        return;

    voiceWriter->EndCommandPacket(trailingBits);
}

void SourceGameContext::OnNetPacket(NetPacket& packet)
{
    if(packet.type == NetMsg::svc_ServerInfo)
    {
        NetMsg::SVC_ServerInfo* serverInfo = static_cast<NetMsg::SVC_ServerInfo*>(packet.data);
        fTickInterval = serverInfo->tickInterval;
        fTickRate = 1.f / fTickInterval;
        logic->OnServerInfo(serverInfo);
    }

    else if(packet.type == NetMsg::svc_UserMessage)
    {
        NetMsg::SVC_UserMessage* umsg = static_cast<NetMsg::SVC_UserMessage*>(packet.data);

        bf_read msg(umsg->data.get(), math::BitsToBytes(umsg->dataLengthInBits));

        if(umsg->msgType == UserMsg::SayText2)
        {
            int client = msg.ReadByte() - 1;
            bool bWantsToChat = msg.ReadByte();

            char msgName[2048] = {0};
            char msgSender[2048] = {0};
            char msgText[2048] = {0};

            msg.ReadString(msgName, sizeof(msgName));
            msg.ReadString(msgSender, sizeof(msgSender));
            msg.ReadString(msgText, sizeof(msgText));

            logic->OnClientChat(client, bWantsToChat, msgName, msgSender, msgText);
        }
    }

    else if(packet.type == NetMsg::svc_VoiceInit || packet.type == NetMsg::svc_VoiceData)
    {
        voiceWriter->OnNetPacket(packet);
    }
}

void SourceGameContext::OnGameEvent(const char *name, GameEvents::EventDataMap &data)
{
    // GameEvents::PrintEvent(name, data);

    if (strcmp(name, "player_disconnect") == 0)
    {
        int userid = data["userid"].i16Value;
        int client = userIdLookUp[userid];

        // player_disconnect can fire for clients which never connected
        // (ESC during mapchange)
        if(client != 0xFF)
        {
            auto& p = players[client];
            assert(p.connected && p.info.userID == userid);

            p.connected = false;
            logic->OnClientDisconnected(client, data["reason"].strValue.c_str());
            userIdLookUp[userid] = 0xFF;
        }
    }

    else if (strcmp(name, "player_death") == 0)
    {
        int client = userIdLookUp[data["userid"].i16Value];
        assert(client >= 0 && client < MAX_PLAYERS);

        int attacker = data["attacker"].i16Value;
        if(attacker > 0)
        {
            attacker = userIdLookUp[attacker];
            assert(attacker >= 0 && attacker < MAX_PLAYERS);
        }
        else
            attacker = -1;

        logic->OnClientDeath(client, attacker, data["headshot"].bValue, data["weapon"].strValue.c_str());
    }

    else if (strcmp(name, "round_start") == 0)
    {
        logic->OnRoundStart(data["timelimit"].i32Value);
    }
    else if (strcmp(name, "round_end") == 0)
    {
        logic->OnRoundEnd(data["message"].strValue.c_str(), data["reason"].u8Value, data["winner"].u8Value);
    }
}

void SourceGameContext::OnStringtable(StringTable* table)
{
    if (table->tableName == "userinfo")
    {
        table->callback = std::bind(&SourceGameContext::UserInfoChanged, this, std::placeholders::_1, std::placeholders::_2);
    }
}

void SourceGameContext::UserInfoChanged(int tableIdx, int entryIdx)
{
    StringTableEntry &entry = stringTables->tables[tableIdx].entries[entryIdx];

    int client = std::stoi(entry.string);
    assert(client >= 0 && client < MAX_PLAYERS);
    player_info_t *info = (player_info_t *)entry.data.data();

    if (entry.data.size() != sizeof(player_info_t))
    {
        if(players[client].connected)
            userIdLookUp[players[client].info.userID] = 0xFF;

        memset(&players[client].info, 0, sizeof(player_info_t));
        players[client].connected = false;
        return;
    }

    memcpy(&players[client].info, info, sizeof(player_info_t));
    userIdLookUp[info->userID] = client;

    if (!players[client].connected)
        logic->OnClientConnected(client);
    else
        logic->OnClientSettingsChanged(client);

    players[client].connected = true;

    //std::cout << client << " (" << info->userID << "): N:" << info->name << " G:" << info->guid << " F:" << info->friendsID << "\n";
}
