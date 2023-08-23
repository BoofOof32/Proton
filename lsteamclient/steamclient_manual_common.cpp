extern "C" {
#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(steamclient);
}

#include "steam_defs.h"
#pragma push_macro("__cdecl")
#undef __cdecl
#include "steamworks_sdk_153a/steam_api.h"
#include "steamworks_sdk_153a/steamnetworkingtypes.h"

#pragma pop_macro("__cdecl")
#include "steamclient_private.h"

extern "C" {
#define SDKVER_153a
#include "struct_converters.h"
#include "cb_converters.h"
#include "win_constructors.h"

#define SDK_VERSION 1531
#include "steamclient_manual_common.h"

struct msg_wrapper {
    struct winSteamNetworkingMessage_t_153a win_msg;
    struct SteamNetworkingMessage_t *lin_msg;

    void (*orig_FreeData)(SteamNetworkingMessage_t *);
};

/***** manual struct converter for SteamNetworkingMessage_t *****/

static void __attribute__((ms_abi)) win_FreeData(struct winSteamNetworkingMessage_t_153a *win_msg)
{
    struct msg_wrapper *msg = CONTAINING_RECORD(win_msg, struct msg_wrapper, win_msg);
    TRACE("%p\n", msg);
    if(msg->orig_FreeData)
    {
        msg->lin_msg->m_pData = msg->win_msg.m_pData;
        msg->orig_FreeData(msg->lin_msg);
    }
}

static void __attribute__((ms_abi)) win_Release(struct winSteamNetworkingMessage_t_153a *win_msg)
{
    struct msg_wrapper *msg = CONTAINING_RECORD(win_msg, struct msg_wrapper, win_msg);

    TRACE("%p\n", msg);
    msg->lin_msg->m_pfnRelease(msg->lin_msg);
    SecureZeroMemory(msg, sizeof(*msg));
    HeapFree(GetProcessHeap(), 0, msg);
}

static void lin_FreeData(struct SteamNetworkingMessage_t *lin_msg)
{
    struct msg_wrapper *msg = (struct msg_wrapper *)lin_msg->m_pData; /* ! see assignment, below */
    TRACE("%p\n", msg);
    if(msg->win_msg.m_pfnFreeData)
        ((void (__attribute__((ms_abi))*)(struct winSteamNetworkingMessage_t_153a *))msg->win_msg.m_pfnFreeData)(&msg->win_msg);
}

void *network_message_lin_to_win_(void *msg_, unsigned int version)
{
    struct SteamNetworkingMessage_t *lin_msg = (struct SteamNetworkingMessage_t *)msg_;
    struct msg_wrapper *msg;

    msg = (struct msg_wrapper *)HeapAlloc(GetProcessHeap(), 0, sizeof(*msg));

    TRACE("lin_msg %p, msg %p.\n", lin_msg, msg);

    msg->lin_msg = lin_msg;

    msg->win_msg.m_pData = msg->lin_msg->m_pData;
    msg->win_msg.m_cbSize = msg->lin_msg->m_cbSize;
    msg->win_msg.m_conn = msg->lin_msg->m_conn;
    msg->win_msg.m_identityPeer = msg->lin_msg->m_identityPeer;
    msg->win_msg.m_nConnUserData = msg->lin_msg->m_nConnUserData;
    msg->win_msg.m_usecTimeReceived= msg->lin_msg->m_usecTimeReceived;
    msg->win_msg.m_nMessageNumber = msg->lin_msg->m_nMessageNumber;
    msg->win_msg.m_pfnFreeData = (void*)win_FreeData;
    msg->win_msg.m_pfnRelease = (void*)win_Release;
    msg->win_msg.m_nChannel = msg->lin_msg->m_nChannel;
    if (version >= 1470)
    {
        msg->win_msg.m_nFlags = msg->lin_msg->m_nFlags;
        msg->win_msg.m_nUserData = msg->lin_msg->m_nUserData;
    }
    if (version >= 1530)
        msg->win_msg.m_idxLane = msg->lin_msg->m_idxLane;

    msg->orig_FreeData = msg->lin_msg->m_pfnFreeData;
    msg->lin_msg->m_pfnFreeData = lin_FreeData;
    /* ! store the wrapper here and restore the original pointer from win_msg before calling orig_FreeData */
    msg->lin_msg->m_pData = msg;

    return &msg->win_msg;
}

void lin_to_win_struct_SteamNetworkingMessage_t_(int n_messages, void **l, void **w, int max_messages, int version)
{
    int i;

    if(n_messages > 0)
        TRACE("%u %p %p\n", n_messages, l, w);

    for(i = 0; i < n_messages; ++i)
        w[i] = network_message_lin_to_win_(l[i], version);

    for(; i < max_messages; ++i)
        w[i] = NULL;
}

void *network_message_win_to_lin_(void *win_msg, unsigned int version)
{
    struct msg_wrapper *msg = CONTAINING_RECORD(win_msg, struct msg_wrapper, win_msg);
    SteamNetworkingMessage_t *lin_msg = msg->lin_msg;

    TRACE("msg %p, lin_msg %p.\n", msg, lin_msg);

    lin_msg->m_pData = msg->win_msg.m_pData;
    lin_msg->m_cbSize = msg->win_msg.m_cbSize;
    lin_msg->m_conn = msg->win_msg.m_conn;
    lin_msg->m_identityPeer = msg->win_msg.m_identityPeer;
    lin_msg->m_nConnUserData = msg->win_msg.m_nConnUserData;
    lin_msg->m_usecTimeReceived= msg->win_msg.m_usecTimeReceived;
    lin_msg->m_nMessageNumber = msg->win_msg.m_nMessageNumber;
    lin_msg->m_nChannel = msg->win_msg.m_nChannel;
    if (version >= 1470)
    {
        lin_msg->m_nFlags = msg->win_msg.m_nFlags;
        lin_msg->m_nUserData = msg->win_msg.m_nUserData;
    }
    if (version >= 1530)
        lin_msg->m_idxLane = msg->win_msg.m_idxLane;

    return lin_msg;
}

} /* extern "C" { */
