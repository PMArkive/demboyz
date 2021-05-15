
#pragma once

#include "base/json.hpp"
#include "game/sourcecontext.h"

using nlohmann::json;

namespace NetMsg
{
    struct SVC_ServerInfo;
}

struct Logic
{
    Logic(SourceGameContext* context);
    ~Logic();

    void Start();
    void Finish(bool dirty);

    struct
    {
        int32_t connected = -1;
        int kills = 0;
        int deaths = 0;
        int chats = 0;
        float voiceTime = 0.0f;
        std::string name;
        int32_t nametime = -1;
    } clients[MAX_PLAYERS];

    void OnServerInfo(NetMsg::SVC_ServerInfo* serverInfo);
    void OnClientConnected(int client);
    void OnClientDisconnected(int client, const char* reason);
    void OnClientSettingsChanged(int client);
    void OnClientDeath(int client, int attacker, bool headshot, const char* weapon);
    void OnClientChat(int client, bool bWantsToChat, const char* msgName, const char* msgSender, const char* msgText);
    void OnClientVoiceChat(int client, float length);
    void OnVoiceCodec(const char* codec, int quality, int sampleRate);
    void OnRoundStart(int timelimit);
    void OnRoundEnd(const char *message, int reason, int winner);

    int32_t curTick = 0;
    float voiceTotalTime = 0.0f;
    float voiceActiveTime = 0.0f;
    float voiceEndTime = 0.0f;
    SourceGameContext* context = nullptr;
    json data;
};
