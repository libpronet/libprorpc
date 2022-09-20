/*
 * Copyright (C) 2018-2019 Eric Tung <libpronet@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of LibProRpc (https://github.com/libpronet/libprorpc)
 */

#include "pro_rpc.h"
#include "rpc_client.h"
#include "rpc_packet.h"
#include "rpc_server.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include <cassert>

#if defined(__cplusplus)
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
////

PRO_RPC_API
IRpcClient*
CreateRpcClient(IRpcClientObserver* observer,
                IProReactor*        reactor,
                const char*         configFileName,
                RTP_MM_TYPE         mmType,     /* = 0 */
                const char*         serverIp,   /* = NULL */
                unsigned short      serverPort, /* = 0 */
                const RTP_MSG_USER* user,       /* = NULL */
                const char*         password,   /* = NULL */
                const char*         localIp)    /* = NULL */
{
    ProRtpInit();

    CRpcClient* const client = CRpcClient::CreateInstance();
    if (client == NULL)
    {
        return (NULL);
    }

    if (!client->Init(observer, reactor, configFileName,
        mmType, serverIp, serverPort, user, password, localIp))
    {
        client->Release();

        return (NULL);
    }

    return (client);
}

PRO_RPC_API
void
DeleteRpcClient(IRpcClient* client)
{
    if (client == NULL)
    {
        return;
    }

    CRpcClient* const p = (CRpcClient*)client;
    p->Fini();
    p->Release();
}

PRO_RPC_API
IRpcServer*
CreateRpcServer(IRpcServerObserver* observer,
                IProReactor*        reactor,
                const char*         configFileName,
                RTP_MM_TYPE         mmType,         /* = 0 */
                unsigned short      serviceHubPort) /* = 0 */
{
    ProRtpInit();

    CRpcServer* const server = CRpcServer::CreateInstance();
    if (server == NULL)
    {
        return (NULL);
    }

    if (!server->Init(
        observer, reactor, configFileName, mmType, serviceHubPort))
    {
        server->Release();

        return (NULL);
    }

    return (server);
}

PRO_RPC_API
void
DeleteRpcServer(IRpcServer* server)
{
    if (server == NULL)
    {
        return;
    }

    CRpcServer* const p = (CRpcServer*)server;
    p->Fini();
    p->Release();
}

PRO_RPC_API
IRpcPacket*
CreateRpcRequest(PRO_UINT32          functionId,
                 const RPC_ARGUMENT* args,  /* = NULL */
                 size_t              count) /* = 0 */
{
    CRpcPacket* packet = CRpcPacket::CreateInstance(functionId, false);
    if (packet == NULL)
    {
        return (NULL);
    }

    if (args != NULL && count > 0)
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->PushArguments(args, count) ||
            !packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }
    else
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }

    return (packet);
}

PRO_RPC_API
IRpcPacket*
CreateRpcResult(PRO_UINT64          clientId,
                PRO_UINT64          requestId,
                PRO_UINT32          functionId,
                RPC_ERROR_CODE      rpcCode,
                const RPC_ARGUMENT* args,  /* = NULL */
                size_t              count) /* = 0 */
{
    assert(clientId > 0);
    if (clientId == 0)
    {
        return (NULL);
    }

    CRpcPacket* packet =
        CRpcPacket::CreateInstance(requestId, functionId, false);
    if (packet == NULL)
    {
        return (NULL);
    }

    packet->SetClientId(clientId);
    packet->SetRpcCode(rpcCode);

    if (args != NULL && count > 0)
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->PushArguments(args, count) ||
            !packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }
    else
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }

    return (packet);
}

PRO_RPC_API
IRpcPacket*
ParseRpcStreamToPacket(const void* streamBuffer,
                       size_t      streamSize)
{
    RPC_HDR                     hdr;
    CProStlVector<RPC_ARGUMENT> args;

    if (!CRpcPacket::ParseRpcPacket(streamBuffer, streamSize, hdr, args))
    {
        return (NULL);
    }

    CRpcPacket* packet = CRpcPacket::CreateInstance(
        hdr.requestId,
        hdr.functionId,
        true /* this is a rebuilt packet */
        );
    if (packet == NULL)
    {
        return (NULL);
    }

    packet->SetRpcCode(hdr.rpcCode);

    if (args.size() > 0)
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->PushArguments(&args[0], args.size()) ||
            !packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }
    else
    {
        packet->CleanAndBeginPushArgument();
        if (!packet->EndPushArgument())
        {
            packet->Release();
            packet = NULL;
        }
    }

    return (packet);
}

/////////////////////////////////////////////////////////////////////////////
////

#if defined(__cplusplus)
} /* extern "C" */
#endif
