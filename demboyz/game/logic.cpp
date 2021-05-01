
#include "logic.h"
#include "netmessages/svc_serverinfo.h"
#include <cstring>
#include <iostream>


Logic::Logic(SourceGameContext* context):
    context(context)
{
    //memset(clients, 0, sizeof(clients));
    data = json({
        {"header", {}},
        {"serverinfo", {}},
        {"players", {}},
        {"chat", {}}
    });
}

Logic::~Logic()
{
}

void Logic::Start()
{
    data["demoheader"] = json({
        {"demofilestamp", context->header.demofilestamp},
        {"demoprotocol", context->header.demoprotocol},
        {"networkprotocol", context->header.networkprotocol},
        {"servername", context->header.servername},
        {"clientname", context->header.clientname},
        {"mapname", context->header.mapname},
        {"gamedirectory", context->header.gamedirectory},
        {"playback_time", context->header.playback_time},
        {"playback_ticks", context->header.playback_ticks},
        {"playback_frames", context->header.playback_frames},
        {"signonlength", context->header.signonlength}
    });

    // std::cout << data.dump(2, ' ', false, json::error_handler_t::replace) << "\n";
}

void Logic::Finish(bool dirty)
{
    // disconnect all remaining clients
    for (int client = 0; client < MAX_PLAYERS; client++)
    {
        if (clients[client].connected == -1)
            continue;

        OnClientDisconnected(client, "#demoend");
    }

    // fix header if demo is corrupt
    data["demoheader"]["dirty"] = dirty;
    if (dirty)
    {
        data["demoheader"]["playback_ticks"] = curTick;
        data["demoheader"]["playback_time"] = curTick * context->fTickRate;
        data["demoheader"]["playback_frames"] = context->curFrame;
    }

    std::string out = data.dump(2, ' ', false, json::error_handler_t::replace);
    out.append("\n");
    fwrite(out.c_str(), out.size(), 1, context->outputFp);
}

void Logic::OnServerInfo(NetMsg::SVC_ServerInfo* serverInfo)
{
    char mapMD5[32+1];
    for (int i = 0; i < 16; ++i)
    {
        mapMD5[(i * 2)] = "0123456789ABCDEF"[serverInfo->mapMD5[i] / 16];
        mapMD5[(i * 2) + 1] = "0123456789ABCDEF"[serverInfo->mapMD5[i] % 16];
    }
    mapMD5[32] = 0;

    json info = {
        {"protocol", serverInfo->protocol},
        {"serverCount", serverInfo->serverCount},
        {"isHLTV", serverInfo->isHLTV},
        {"isDedicated", serverInfo->isDedicated},
        {"clientCRC", serverInfo->clientCRC},
        {"maxClasses", serverInfo->maxClasses},
        {"mapCRC", serverInfo->mapCRC},
        {"mapMD5", mapMD5},
        {"playerSlot", serverInfo->playerSlot},
        {"maxClients", serverInfo->maxClients},
        {"tickInterval", serverInfo->tickInterval},
        {"os", serverInfo->os},
        {"gameDir", serverInfo->gameDir},
        {"mapName", serverInfo->mapName},
        {"skyName", serverInfo->skyName},
        {"hostName", serverInfo->hostName},
        {"isReplay", serverInfo->isReplay}
    };
    data["serverinfo"] = info;

    // std::cout << info.dump(2, ' ', false, json::error_handler_t::replace) << "\n";
}

void Logic::OnClientConnected(int client)
{
    assert(clients[client].connected == -1);

    const auto& info = context->players[client].info;
    auto& player = data["players"][info.guid];

    if (player.is_null())
    {
        player = json({
            {"names", {}},
            {"sprays", {}},
            {"disconnect_reasons", {}},
            {"playtime", 0},
            {"voicetime", 0.0f}
        });
    }

    clients[client].connected = curTick;

    OnClientSettingsChanged(client);
}

void Logic::OnClientDisconnected(int client, const char* reason)
{
    assert(clients[client].connected != -1);

    const auto& info = context->players[client].info;
    auto& player = data["players"][info.guid];

    // cumulative play time
    unsigned int playtime = curTick - clients[client].connected;
    playtime += player["playtime"].get<unsigned int>();
    player["playtime"] = playtime;

    // cumulative voice chat time
    float voicetime = clients[client].voiceTime;
    voicetime += player["voicetime"].get<float>();
    player["voicetime"] = voicetime;

    player["disconnect_reasons"] += reason;

    clients[client].connected = -1;
}

void Logic::OnClientSettingsChanged(int client)
{
    assert(clients[client].connected != -1);

    const auto& info = context->players[client].info;
    auto& player = data["players"][info.guid];

    // list of names
    if (std::find(player["names"].begin(), player["names"].end(), info.name) == player["names"].end())
        player["names"].push_back(info.name);

    // list of spray hashes
    if (info.customFiles[0])
    {
        char logohex[16];
        sprintf(logohex, "%08x", info.customFiles[0]);
        if (std::find(player["sprays"].begin(), player["sprays"].end(), logohex) == player["sprays"].end())
            player["sprays"].push_back(logohex);
    }
}

void Logic::OnClientChat(int client, bool bWantsToChat, const char* msgName, const char* msgSender, const char* msgText)
{
    const auto& info = context->players[client].info;
    json chat = {
        {"tick", curTick},
        {"steamid", info.guid},
        {"msgName", msgName},
        {"msgSender", msgSender},
        {"msgText", msgText}
    };

    data["chat"] += chat;
}

void Logic::OnClientVoiceChat(int client, float length)
{
    assert(clients[client].connected != -1);

    clients[client].voiceTime += length;
}

void Logic::OnVoiceCodec(const char* codec, int quality, int sampleRate)
{
    data["voice_init"] = json({
        {"codec", codec},
        {"quality", quality},
        {"sampleRate", sampleRate}
    });
}