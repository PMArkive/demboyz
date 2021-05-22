
#include "logic.h"
#include "netmessages/svc_serverinfo.h"
#include "io/voicewriter/voicedatawriter.h"
#include <cstring>
#include <iostream>


Logic::Logic(SourceGameContext* context):
    context(context)
{
    data = json({
        {"demoheader", {}},
        {"serverinfo", {}},
        {"players", {}},
        {"events", {}},
        {"chat", {}},
        {"voice", {}}
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
        data["demoheader"]["playback_time"] = curTick * context->fTickInterval;
        data["demoheader"]["playback_frames"] = context->curFrame;
    }

    data["voice"]["total_time"] = voiceTotalTime;
    data["voice"]["active_time"] = voiceActiveTime;

    for(const auto& o : context->voiceWriter->m_silence)
    {
        data["voice"]["silence"] += json({o.first, o.second});
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
    assert(client >= 0 && client < MAX_PLAYERS);
    assert(clients[client].connected == -1);

    const auto& info = context->players[client].info;
    auto& player = data["players"][info.guid];

    if (player.is_null())
    {
        player = json({
            {"names", {}},
            {"sprays", {}},
            {"playtime", 0},
            {"voicetime", 0.0f},
            {"kills", 0},
            {"deaths", 0},
            {"chats", 0}
        });
    }

    clients[client].connected = curTick;
    clients[client].kills = 0;
    clients[client].deaths = 0;
    clients[client].chats = 0;
    clients[client].voiceTime = 0.0f;
    clients[client].name.assign(info.name);
    clients[client].nametime = curTick;

    json player_connect = {
        {"event", "player_connect"},
        {"tick", curTick},
        {"steamid", info.guid},
        {"name", info.name}
    };
    data["events"] += player_connect;

    OnClientSettingsChanged(client);
}

void Logic::OnClientDisconnected(int client, const char* reason)
{
    assert(client >= 0 && client < MAX_PLAYERS);
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

    // cumulative kills
    int kills = clients[client].kills;
    kills += player["kills"].get<int>();
    player["kills"] = kills;

    // cumulative deaths
    int deaths = clients[client].deaths;
    deaths += player["deaths"].get<int>();
    player["deaths"] = deaths;

    // cumulative chats
    int chats = clients[client].chats;
    chats += player["chats"].get<int>();
    player["chats"] = chats;

    // name usage
    unsigned int hadName = curTick - clients[client].nametime;
    if (player["names"].contains(clients[client].name))
        hadName += player["names"][clients[client].name].get<unsigned int>();
    player["names"][clients[client].name] = hadName;

    json player_disconnect = {
        {"event", "player_disconnect"},
        {"tick", curTick},
        {"steamid", info.guid},
        {"reason", reason}
    };
    data["events"] += player_disconnect;

    clients[client].connected = -1;
}

void Logic::OnClientSettingsChanged(int client)
{
    assert(client >= 0 && client < MAX_PLAYERS);
    assert(clients[client].connected != -1);

    const auto& info = context->players[client].info;
    auto& player = data["players"][info.guid];

    // name usage
    if (clients[client].name != info.name)
    {
        unsigned int hadName = curTick - clients[client].nametime;
        if (player["names"].contains(clients[client].name))
            hadName += player["names"][clients[client].name].get<unsigned int>();
        player["names"][clients[client].name] = hadName;

        json player_changename = {
            {"event", "player_changename"},
            {"tick", curTick},
            {"steamid", info.guid},
            {"oldname", clients[client].name},
            {"newname", info.name}
        };
        data["events"] += player_changename;

        clients[client].name.assign(info.name);
        clients[client].nametime = curTick;
    }

    // list of spray hashes
    if (info.customFiles[0])
    {
        char logohex[16];
        sprintf(logohex, "%08x", info.customFiles[0]);
        if (std::find(player["sprays"].begin(), player["sprays"].end(), logohex) == player["sprays"].end())
            player["sprays"].push_back(logohex);
    }
}

void Logic::OnClientDeath(int client, int attacker, bool headshot, const char* weapon)
{
    assert(client >= 0 && client < MAX_PLAYERS);
    assert(clients[client].connected != -1);

    const char *victim_guid = context->players[client].info.guid;
    const char *attacker_guid = "";

    clients[client].deaths++;
    if(attacker >= 0 && attacker < MAX_PLAYERS)
    {
        clients[attacker].kills++;
        attacker_guid = context->players[attacker].info.guid;
    }

    json player_death = {
        {"tick", curTick},
        {"event", "player_death"},
        {"victim", victim_guid},
        {"attacker", attacker_guid},
        {"headshot", headshot},
        {"weapon", weapon}
    };

    data["events"] += player_death;
}

void Logic::OnClientChat(int client, bool bWantsToChat, const char* msgName, const char* msgSender, const char* msgText)
{
    //assert(client >= 0 && client < MAX_PLAYERS);
    //assert(clients[client].connected != -1);
    if(client < 0 || client > MAX_PLAYERS || clients[client].connected == -1)
        return;

    clients[client].chats++;

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
    if(client < 0 || client > MAX_PLAYERS || clients[client].connected == -1)
        return;

    clients[client].voiceTime += length;
    voiceTotalTime += length;

    float now = curTick * context->fTickRate;
    float endtime = now + length;
    if (now >= voiceEndTime)
    {
        voiceEndTime = endtime;
        voiceActiveTime += length;
    }
    else if (endtime > voiceEndTime)
    {
        voiceActiveTime += (endtime - voiceEndTime);
        voiceEndTime = endtime;
    }
}

void Logic::OnVoiceCodec(const char* codec, int quality, int sampleRate)
{
    data["voice"] = json({
        {"codec", codec},
        {"quality", quality},
        {"sampleRate", sampleRate}
    });
}

void Logic::OnRoundStart(int timelimit)
{
    json round_start = {
        {"event", "round_start"},
        {"tick", curTick},
        {"timelimit", timelimit},
    };

    data["events"] += round_start;
}

void Logic::OnRoundEnd(const char *message, int reason, int winner)
{
    json round_end = {
        {"event", "round_end"},
        {"tick", curTick},
        {"message", message},
        {"reason", reason},
        {"winner", winner}
    };

    data["events"] += round_end;
}