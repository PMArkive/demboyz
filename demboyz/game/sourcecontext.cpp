
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

#include "netmessages/svc_voiceinit.h"
#include "netmessages/svc_voicedata.h"

SourceGameContext::SourceGameContext(std::string outputDir, std::string outputDirVoice):
    outputDir(outputDir),
    outputDirVoice(outputDirVoice)
{
    stringTables = new StringTableContainer(this);
    memset(players, 0, sizeof(players));
}

SourceGameContext::~SourceGameContext()
{
    delete logic;
    logic = nullptr;

    protocol = 0;
    delete gameEventList;
    gameEventList = nullptr;
    delete stringTables;
    stringTables = nullptr;

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
    logic->Finish(dirty);
    voiceWriter->Finish();
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
            int client = msg.ReadByte();
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
        for (int client = 0; client < MAX_PLAYERS; client++)
        {
            auto& p = players[client];
            if (!p.connected || p.info.userID != userid)
                continue;

            p.connected = false;
            logic->OnClientDisconnected(client, data["reason"].strValue.c_str());
        }
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
    player_info_t *info = (player_info_t *)entry.data.data();

    if (entry.data.size() != sizeof(player_info_t))
    {
        memset(&players[client].info, 0, sizeof(player_info_t));
        players[client].connected = false;
        return;
    }

    memcpy(&players[client].info, info, sizeof(player_info_t));

    if (!players[client].connected)
        logic->OnClientConnected(client);
    else
        logic->OnClientSettingsChanged(client);

    players[client].connected = true;

    //std::cout << client << " (" << info->userID << "): N:" << info->name << " G:" << info->guid << " F:" << info->friendsID << "\n";
}
