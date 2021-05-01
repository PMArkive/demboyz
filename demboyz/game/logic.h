
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
        float voiceTime = 0.0f;
    } clients[MAX_PLAYERS];

    void OnServerInfo(NetMsg::SVC_ServerInfo* serverInfo);
    void OnClientConnected(int client);
    void OnClientDisconnected(int client, const char* reason);
    void OnClientSettingsChanged(int client);
    void OnClientChat(int client, bool bWantsToChat, const char* msgName, const char* msgSender, const char* msgText);
    void OnClientVoiceChat(int client, float length);
    void OnVoiceCodec(const char* codec, int quality, int sampleRate);

    int32_t curTick = 0;
    SourceGameContext* context = nullptr;
    json data;
};
