
#include "nethandlers.h"
#include "netmessages.h"
#include <cassert>
#include <cstdint>

#if !defined( MAX_OSPATH )
#define MAX_OSPATH      260         // max length of a filesystem pathname
#endif

#include "net_nop.h"
#include "net_disconnect.h"
#include "net_file.h"
#include "net_tick.h"
#include "net_stringcmd.h"
#include "net_setconvar.h"
#include "net_signonstate.h"
#include "svc_print.h"
#include "svc_serverinfo.h"
#include "svc_sendtable.h"
#include "svc_classinfo.h"
#include "svc_setpause.h"
#include "svc_createstringtable.h"
#include "svc_updatestringtable.h"
#include "svc_voiceinit.h"
#include "svc_voicedata.h"
#include "svc_hltv.h"
#include "svc_sounds.h"
#include "svc_setview.h"
#include "svc_fixangle.h"
#include "svc_crosshairangle.h"
#include "svc_bspdecal.h"
#include "svc_terrainmod.h"
#include "svc_usermessage.h"
#include "svc_entitymessage.h"
#include "svc_gameevent.h"
#include "svc_packetentities.h"
#include "svc_tempentities.h"
#include "svc_prefetch.h"
#include "svc_menu.h"
#include "svc_gameeventlist.h"
#include "svc_getcvarvalue.h"
#include "svc_cmdkeyvalues.h"
#include "svc_setpausetimed.h"

void NetHandlers::CreateNetMsgStructs(NetDataStructArray& netDataStructs)
{
    netDataStructs[NetMsg::net_NOP] = new NetMsg::Net_NOP();
    netDataStructs[NetMsg::net_Disconnect] = new NetMsg::Net_Disconnect();
    netDataStructs[NetMsg::net_File] = new NetMsg::Net_File();
    netDataStructs[NetMsg::net_Tick] = new NetMsg::Net_Tick();
    netDataStructs[NetMsg::net_StringCmd] = new NetMsg::Net_StringCmd();
    netDataStructs[NetMsg::net_SetConVar] = new NetMsg::Net_SetConVar();
    netDataStructs[NetMsg::net_SignonState] = new NetMsg::Net_SignonState();
    netDataStructs[NetMsg::svc_Print] = new NetMsg::SVC_Print();
    netDataStructs[NetMsg::svc_ServerInfo] = new NetMsg::SVC_ServerInfo();
    netDataStructs[NetMsg::svc_SendTable] = new NetMsg::SVC_SendTable();
    netDataStructs[NetMsg::svc_ClassInfo] = new NetMsg::SVC_ClassInfo();
    netDataStructs[NetMsg::svc_SetPause] = new NetMsg::SVC_SetPause();
    netDataStructs[NetMsg::svc_CreateStringTable] = new NetMsg::SVC_CreateStringTable();
    netDataStructs[NetMsg::svc_UpdateStringTable] = new NetMsg::SVC_UpdateStringTable();
    netDataStructs[NetMsg::svc_VoiceInit] = new NetMsg::SVC_VoiceInit();
    netDataStructs[NetMsg::svc_VoiceData] = new NetMsg::SVC_VoiceData();
    netDataStructs[NetMsg::svc_HLTV] = new NetMsg::SVC_HLTV();
    netDataStructs[NetMsg::svc_Sounds] = new NetMsg::SVC_Sounds();
    netDataStructs[NetMsg::svc_SetView] = new NetMsg::SVC_SetView();
    netDataStructs[NetMsg::svc_FixAngle] = new NetMsg::SVC_FixAngle();
    netDataStructs[NetMsg::svc_CrosshairAngle] = new NetMsg::SVC_CrosshairAngle();
    netDataStructs[NetMsg::svc_BSPDecal] = new NetMsg::SVC_BSPDecal();
    netDataStructs[NetMsg::svc_TerrainMod] = new NetMsg::SVC_TerrainMod();
    netDataStructs[NetMsg::svc_UserMessage] = new NetMsg::SVC_UserMessage();
    netDataStructs[NetMsg::svc_EntityMessage] = new NetMsg::SVC_EntityMessage();
    netDataStructs[NetMsg::svc_GameEvent] = new NetMsg::SVC_GameEvent();
    netDataStructs[NetMsg::svc_PacketEntities] = new NetMsg::SVC_PacketEntities();
    netDataStructs[NetMsg::svc_TempEntities] = new NetMsg::SVC_TempEntities();
    netDataStructs[NetMsg::svc_Prefetch] = new NetMsg::SVC_Prefetch();
    netDataStructs[NetMsg::svc_Menu] = new NetMsg::SVC_Menu();
    netDataStructs[NetMsg::svc_GameEventList] = new NetMsg::SVC_GameEventList();
    netDataStructs[NetMsg::svc_GetCvarValue] = new NetMsg::SVC_GetCvarValue();
    netDataStructs[NetMsg::svc_CmdKeyValues] = new NetMsg::SVC_CmdKeyValues();
    netDataStructs[NetMsg::svc_SetPauseTimed] = new NetMsg::SVC_SetPauseTimed();
}

void NetHandlers::DestroyNetMsgStructs(NetDataStructArray& netDataStructs)
{
    delete reinterpret_cast<NetMsg::Net_NOP*>(netDataStructs[NetMsg::net_NOP]);
    delete reinterpret_cast<NetMsg::Net_Disconnect*>(netDataStructs[NetMsg::net_Disconnect]);
    delete reinterpret_cast<NetMsg::Net_File*>(netDataStructs[NetMsg::net_File]);
    delete reinterpret_cast<NetMsg::Net_Tick*>(netDataStructs[NetMsg::net_Tick]);
    delete reinterpret_cast<NetMsg::Net_StringCmd*>(netDataStructs[NetMsg::net_StringCmd]);
    delete reinterpret_cast<NetMsg::Net_SetConVar*>(netDataStructs[NetMsg::net_SetConVar]);
    delete reinterpret_cast<NetMsg::Net_SignonState*>(netDataStructs[NetMsg::net_SignonState]);
    delete reinterpret_cast<NetMsg::SVC_Print*>(netDataStructs[NetMsg::svc_Print]);
    delete reinterpret_cast<NetMsg::SVC_ServerInfo*>(netDataStructs[NetMsg::svc_ServerInfo]);
    delete reinterpret_cast<NetMsg::SVC_SendTable*>(netDataStructs[NetMsg::svc_SendTable]);
    delete reinterpret_cast<NetMsg::SVC_ClassInfo*>(netDataStructs[NetMsg::svc_ClassInfo]);
    delete reinterpret_cast<NetMsg::SVC_SetPause*>(netDataStructs[NetMsg::svc_SetPause]);
    delete reinterpret_cast<NetMsg::SVC_CreateStringTable*>(netDataStructs[NetMsg::svc_CreateStringTable]);
    delete reinterpret_cast<NetMsg::SVC_UpdateStringTable*>(netDataStructs[NetMsg::svc_UpdateStringTable]);
    delete reinterpret_cast<NetMsg::SVC_VoiceInit*>(netDataStructs[NetMsg::svc_VoiceInit]);
    delete reinterpret_cast<NetMsg::SVC_VoiceData*>(netDataStructs[NetMsg::svc_VoiceData]);
    delete reinterpret_cast<NetMsg::SVC_HLTV*>(netDataStructs[NetMsg::svc_HLTV]);
    delete reinterpret_cast<NetMsg::SVC_Sounds*>(netDataStructs[NetMsg::svc_Sounds]);
    delete reinterpret_cast<NetMsg::SVC_SetView*>(netDataStructs[NetMsg::svc_SetView]);
    delete reinterpret_cast<NetMsg::SVC_FixAngle*>(netDataStructs[NetMsg::svc_FixAngle]);
    delete reinterpret_cast<NetMsg::SVC_CrosshairAngle*>(netDataStructs[NetMsg::svc_CrosshairAngle]);
    delete reinterpret_cast<NetMsg::SVC_BSPDecal*>(netDataStructs[NetMsg::svc_BSPDecal]);
    delete reinterpret_cast<NetMsg::SVC_TerrainMod*>(netDataStructs[NetMsg::svc_TerrainMod]);
    delete reinterpret_cast<NetMsg::SVC_UserMessage*>(netDataStructs[NetMsg::svc_UserMessage]);
    delete reinterpret_cast<NetMsg::SVC_EntityMessage*>(netDataStructs[NetMsg::svc_EntityMessage]);
    delete reinterpret_cast<NetMsg::SVC_GameEvent*>(netDataStructs[NetMsg::svc_GameEvent]);
    delete reinterpret_cast<NetMsg::SVC_PacketEntities*>(netDataStructs[NetMsg::svc_PacketEntities]);
    delete reinterpret_cast<NetMsg::SVC_TempEntities*>(netDataStructs[NetMsg::svc_TempEntities]);
    delete reinterpret_cast<NetMsg::SVC_Prefetch*>(netDataStructs[NetMsg::svc_Prefetch]);
    delete reinterpret_cast<NetMsg::SVC_Menu*>(netDataStructs[NetMsg::svc_Menu]);
    delete reinterpret_cast<NetMsg::SVC_GameEventList*>(netDataStructs[NetMsg::svc_GameEventList]);
    delete reinterpret_cast<NetMsg::SVC_GetCvarValue*>(netDataStructs[NetMsg::svc_GetCvarValue]);
    delete reinterpret_cast<NetMsg::SVC_CmdKeyValues*>(netDataStructs[NetMsg::svc_CmdKeyValues]);
    delete reinterpret_cast<NetMsg::SVC_SetPauseTimed*>(netDataStructs[NetMsg::svc_SetPauseTimed]);
}

#define DECLARE_NET_HANDLER_ARRAY(funcname) \
    { \
        &NetHandlers::Net_NOP_##funcname, \
        &NetHandlers::Net_Disconnect_##funcname, \
        &NetHandlers::Net_File_##funcname, \
        &NetHandlers::Net_Tick_##funcname, \
        &NetHandlers::Net_StringCmd_##funcname, \
        &NetHandlers::Net_SetConVar_##funcname, \
        &NetHandlers::Net_SignonState_##funcname, \
        &NetHandlers::SVC_Print_##funcname, \
        &NetHandlers::SVC_ServerInfo_##funcname, \
        &NetHandlers::SVC_SendTable_##funcname, \
        &NetHandlers::SVC_ClassInfo_##funcname, \
        &NetHandlers::SVC_SetPause_##funcname, \
        &NetHandlers::SVC_CreateStringTable_##funcname, \
        &NetHandlers::SVC_UpdateStringTable_##funcname, \
        &NetHandlers::SVC_VoiceInit_##funcname, \
        &NetHandlers::SVC_VoiceData_##funcname, \
        &NetHandlers::SVC_HLTV_##funcname, \
        &NetHandlers::SVC_Sounds_##funcname, \
        &NetHandlers::SVC_SetView_##funcname, \
        &NetHandlers::SVC_FixAngle_##funcname, \
        &NetHandlers::SVC_CrosshairAngle_##funcname, \
        &NetHandlers::SVC_BSPDecal_##funcname, \
        &NetHandlers::SVC_TerrainMod_##funcname, \
        &NetHandlers::SVC_UserMessage_##funcname, \
        &NetHandlers::SVC_EntityMessage_##funcname, \
        &NetHandlers::SVC_GameEvent_##funcname, \
        &NetHandlers::SVC_PacketEntities_##funcname, \
        &NetHandlers::SVC_TempEntities_##funcname, \
        &NetHandlers::SVC_Prefetch_##funcname, \
        &NetHandlers::SVC_Menu_##funcname, \
        &NetHandlers::SVC_GameEventList_##funcname, \
        &NetHandlers::SVC_GetCvarValue_##funcname, \
        &NetHandlers::SVC_CmdKeyValues_##funcname, \
        &NetHandlers::SVC_SetPauseTimed_##funcname \
    }

typedef bool (*NetMsgBitReadFn)(NetHandlers::BitRead& bitbuf, SourceGameContext& context, void* data);

template<typename BufType, typename FnType, typename ContextType, size_t NumHandlers>
bool NetMsgFuncRunner(const FnType (&netHandlers)[NumHandlers], uint32_t type, BufType& buf, ContextType& context, void* data)
{
    if (type >= NumHandlers)
    {
        return false;
    }
    return netHandlers[type](buf, context, data);
}

bool NetHandlers::NetMsg_BitRead(uint32_t type, BitRead& bitbuf, SourceGameContext& context, void* data)
{
    static const NetMsgBitReadFn netHandlers[] = DECLARE_NET_HANDLER_ARRAY(BitRead);
    return NetMsgFuncRunner(netHandlers, type, bitbuf, context, data);
}

