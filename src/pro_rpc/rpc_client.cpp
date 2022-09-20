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

#include "rpc_client.h"
#include "pro_rpc.h"
#include "rpc_packet.h"
#include "rpc_server.h"
#include "promsg/msg_client.h"
#include "pronet/pro_config_file.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_net.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_time_util.h"
#include "pronet/pro_timer_factory.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
////

static const RTP_MSG_USER  RPC_ROOT_ID(1, 1, 0); /* 1-1 */
static const unsigned char RPC_CID = 2;
static const PRO_UINT16    RPC_IID = 1;

/////////////////////////////////////////////////////////////////////////////
////

static
void
ReadConfig_i(const CProStlVector<PRO_CONFIG_ITEM>& configs,
             RPC_CLIENT_CONFIG_INFO&               configInfo)
{
    int       i = 0;
    const int c = (int)configs.size();

    for (; i < c; ++i)
    {
        const CProStlString& configName  = configs[i].configName;
        const CProStlString& configValue = configs[i].configValue;

        if (stricmp(configName.c_str(), "rpcc_pending_calls") == 0)
        {
            const int value = atoi(configValue.c_str());
            if (value > 0)
            {
                configInfo.rpcc_pending_calls = value;
            }
        }
        else if (stricmp(configName.c_str(), "rpcc_rpc_timeout") == 0)
        {
            const int value = atoi(configValue.c_str());
            if (value > 0 && value <= 3600)
            {
                configInfo.rpcc_rpc_timeout = value;
            }
        }
        else
        {
        }
    } /* end of for (...) */
}

/////////////////////////////////////////////////////////////////////////////
////

CRpcClient*
CRpcClient::CreateInstance()
{
    CRpcClient* const client = new CRpcClient;

    return (client);
}

CRpcClient::CRpcClient()
{
    m_observer = NULL;
    m_packet   = NULL;
    m_clientId = 0;
    m_magic    = 0;
}

CRpcClient::~CRpcClient()
{
    Fini();
}

bool
CRpcClient::Init(IRpcClientObserver* observer,
                 IProReactor*        reactor,
                 const char*         configFileName,
                 RTP_MM_TYPE         mmType,     /* = 0 */
                 const char*         serverIp,   /* = NULL */
                 unsigned short      serverPort, /* = 0 */
                 const RTP_MSG_USER* user,       /* = NULL */
                 const char*         password,   /* = NULL */
                 const char*         localIp)    /* = NULL */
{
    assert(observer != NULL);
    assert(reactor != NULL);
    assert(configFileName != NULL);
    assert(configFileName[0] != '\0');
    if (observer == NULL || reactor == NULL || configFileName == NULL ||
        configFileName[0] == '\0')
    {
        return (false);
    }

    char exeRoot[1024] = "";
    ProGetExeDir_(exeRoot);

    CProStlString configFileName2 = configFileName;
    if (configFileName2[0] == '.' ||
        configFileName2.find_first_of("\\/") == CProStlString::npos)
    {
        CProStlString fileName = exeRoot;
        fileName += configFileName2;
        configFileName2 = fileName;
    }

    CProConfigFile configFile;
    configFile.Init(configFileName2.c_str());

    CProStlVector<PRO_CONFIG_ITEM> configs;
    if (!configFile.Read(configs))
    {
        return (false);
    }

    RPC_CLIENT_CONFIG_INFO configInfo;
    ReadConfig_i(configs, configInfo);

    CRpcPacket* packet = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        assert(m_observer == NULL);
        assert(m_reactor == NULL);
        assert(m_packet == NULL);
        if (m_observer != NULL || m_reactor != NULL || m_packet != NULL)
        {
            return (false);
        }

        if (!CMsgClient::Init(reactor, configFileName, mmType,
            serverIp, serverPort, user, password, localIp))
        {
            goto EXIT;
        }

        packet = CRpcPacket::CreateInstance();
        if (packet == NULL)
        {
            goto EXIT;
        }

        observer->AddRef();
        m_observer   = observer;
        m_configInfo = configInfo;
        m_packet     = packet;
    }

    return (true);

EXIT:

    if (packet != NULL)
    {
        packet->Release();
    }

    CMsgClient::Fini();

    return (false);
}

void
CRpcClient::Fini()
{
    IRpcClientObserver* observer = NULL;
    CRpcPacket*         packet   = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return;
        }

        CProStlMap<PRO_UINT64, RPC_HDR2>::iterator       itr = m_timerId2Hdr.begin();
        CProStlMap<PRO_UINT64, RPC_HDR2>::iterator const end = m_timerId2Hdr.end();

        for (; itr != end; ++itr)
        {
            m_reactor->CancelTimer(itr->first);
        }

        m_requestId2TimerId.clear();
        m_timerId2Hdr.clear();
        m_funtionId2Info.clear();
        packet = m_packet;
        m_packet = NULL;
        observer = m_observer;
        m_observer = NULL;
    }

    packet->Release();
    observer->Release();

    CMsgClient::Fini();
}

unsigned long
CRpcClient::AddRef()
{
    const unsigned long refCount = CMsgClient::AddRef();

    return (refCount);
}

unsigned long
CRpcClient::Release()
{
    const unsigned long refCount = CMsgClient::Release();

    return (refCount);
}

RTP_MM_TYPE
CRpcClient::GetMmType() const
{
    RTP_MM_TYPE mmType = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        mmType = m_msgConfigInfo.msgc_mm_type;
    }

    return (mmType);
}

PRO_UINT64
CRpcClient::GetClientId() const
{
    PRO_UINT64 clientId = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        clientId = m_clientId;
    }

    return (clientId);
}

const char*
CRpcClient::GetServerIp(char serverIp[64]) const
{
    return (CMsgClient::GetRemoteIp(serverIp));
}

unsigned short
CRpcClient::GetServerPort() const
{
    return (CMsgClient::GetRemotePort());
}

const char*
CRpcClient::GetLocalIp(char localIp[64]) const
{
    return (CMsgClient::GetLocalIp(localIp));
}

unsigned short
CRpcClient::GetLocalPort() const
{
    return (CMsgClient::GetLocalPort());
}

RPC_ERROR_CODE
CRpcClient::RegisterFunction(PRO_UINT32           functionId,
                             const RPC_DATA_TYPE* callArgTypes, /* = NULL */
                             size_t               callArgCount, /* = 0 */
                             const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
                             size_t               retnArgCount) /* = 0 */
{
    assert(functionId > 0);
    if (functionId == 0)
    {
        return (RPCE_INVALID_ARGUMENT);
    }

    if (
        (callArgTypes == NULL && callArgCount > 0)
        ||
        (retnArgTypes == NULL && retnArgCount > 0)
       )
    {
        assert(0);

        return (RPCE_INVALID_ARGUMENT);
    }

    for (int i = 0; i < (int)callArgCount; ++i)
    {
        assert(CheckRpcDataType(callArgTypes[i]));
        if (!CheckRpcDataType(callArgTypes[i]))
        {
            return (RPCE_INVALID_ARGUMENT);
        }
    }

    for (int j = 0; j < (int)retnArgCount; ++j)
    {
        assert(CheckRpcDataType(retnArgTypes[j]));
        if (!CheckRpcDataType(retnArgTypes[j]))
        {
            return (RPCE_INVALID_ARGUMENT);
        }
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return (RPCE_ERROR);
        }

        RPC_FUNCTION_INFO& info = m_funtionId2Info[functionId]; /* insert */
        info.callArgTypes.clear();
        info.retnArgTypes.clear();

        for (int m = 0; m < (int)callArgCount; ++m)
        {
            info.callArgTypes.push_back(callArgTypes[m]);
        }

        for (int n = 0; n < (int)retnArgCount; ++n)
        {
            info.retnArgTypes.push_back(retnArgTypes[n]);
        }
    }

    return (RPCE_OK);
}

void
CRpcClient::UnregisterFunction(PRO_UINT32 functionId)
{
    if (functionId == 0)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return;
        }

        m_funtionId2Info.erase(functionId);
    }
}

RPC_ERROR_CODE
CRpcClient::SendRpcRequest(IRpcPacket*   request,
                           bool          noreply,             /* = false */
                           unsigned long rpcTimeoutInSeconds) /* = 0 */
{
    assert(request != NULL);
    if (request == NULL)
    {
        return (RPCE_INVALID_ARGUMENT);
    }

    if (rpcTimeoutInSeconds == 0)
    {
        rpcTimeoutInSeconds = m_configInfo.rpcc_rpc_timeout;
    }

    CRpcPacket* const request2 = (CRpcPacket*)request;
    request2->SetNoreply(noreply);
    request2->SetTimeout(rpcTimeoutInSeconds);

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return (RPCE_ERROR);
        }

        if (m_msgClient == NULL || m_clientId == 0)
        {
            return (RPCE_NETWORK_NOT_CONNECTED);
        }

        if (m_timerId2Hdr.size() >= m_configInfo.rpcc_pending_calls)
        {
            return (RPCE_CLIENT_BUSY);
        }

        CProStlMap<PRO_UINT32, RPC_FUNCTION_INFO>::iterator const itr =
            m_funtionId2Info.find(request->GetFunctionId());
        if (itr == m_funtionId2Info.end())
        {
            return (RPCE_INVALID_FUNCTION);
        }

        const RPC_FUNCTION_INFO& info = itr->second;
        if (!CmpRpcPacketTypes(request, info.callArgTypes))
        {
            return (RPCE_MISMATCHED_PARAMETER);
        }

        if (!m_msgClient->SendMsg(request->GetTotalBuffer(),
            request->GetTotalSize(), 0, &RPC_ROOT_ID, 1))
        {
            return (RPCE_NETWORK_BUSY);
        }

        if (!noreply)
        {
            RPC_HDR2 hdr;
            memset(&hdr, 0, sizeof(RPC_HDR2));
            hdr.requestId  = request->GetRequestId();
            hdr.functionId = request->GetFunctionId();
            hdr.magic1     = request->GetMagic1();
            hdr.magic2     = request->GetMagic2();

            const PRO_UINT64 timerId = m_reactor->ScheduleTimer(
                this, (PRO_UINT64)rpcTimeoutInSeconds * 1000, false);

            m_timerId2Hdr[timerId]             = hdr;
            m_requestId2TimerId[hdr.requestId] = timerId;
        }
    }

    return (RPCE_OK);
}

bool
CRpcClient::SendMsgToServer(const void*   buf,
                            unsigned long size,
                            PRO_UINT16    charset)
{
    assert(buf != NULL);
    assert(size > 0);
    if (buf == NULL || size == 0)
    {
        return (false);
    }

    bool ret = false;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return (false);
        }

        if (m_msgClient == NULL || m_clientId == 0)
        {
            return (false);
        }

        ret = m_msgClient->SendMsg(buf, size, charset, &RPC_ROOT_ID, 1);
    }

    return (ret);
}

bool
CRpcClient::SendMsgToClients(const void*       buf,
                             unsigned long     size,
                             PRO_UINT16        charset,
                             const PRO_UINT64* dstClients,
                             unsigned char     dstClientCount)
{
    assert(buf != NULL);
    assert(size > 0);
    assert(dstClients != NULL);
    assert(dstClientCount > 0);
    if (buf == NULL || size == 0 || dstClients == NULL || dstClientCount == 0)
    {
        return (false);
    }

    CProStlVector<RTP_MSG_USER> dstUsers;

    for (int i = 0; i < (int)dstClientCount; ++i)
    {
        const RTP_MSG_USER user(RPC_CID, dstClients[i], RPC_IID);
        dstUsers.push_back(user);
    }

    bool ret = false;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return (false);
        }

        if (m_msgClient == NULL || m_clientId == 0)
        {
            return (false);
        }

        ret = m_msgClient->SendMsg(buf, size, charset,
            &dstUsers[0], (unsigned char)dstUsers.size());
    }

    return (ret);
}

bool
CRpcClient::Reconnect()
{
    return (CMsgClient::Reconnect());
}

void
CRpcClient::SetMagic(PRO_INT64 magic)
{
    {
        CProThreadMutexGuard mon(m_lock);

        m_magic = magic;
    }
}

PRO_INT64
CRpcClient::GetMagic() const
{
    PRO_INT64 magic = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        magic = m_magic;
    }

    return (magic);
}

void
CRpcClient::OnOkMsg(IRtpMsgClient*      msgClient,
                    const RTP_MSG_USER* myUser,
                    const char*         myPublicIp)
{
    assert(msgClient != NULL);
    assert(myUser != NULL);
    assert(myPublicIp != NULL);
    assert(myPublicIp[0] != '\0');
    if (msgClient == NULL || myUser == NULL || myPublicIp == NULL ||
        myPublicIp[0] == '\0')
    {
        return;
    }

    const PRO_UINT64 clientId = myUser->UserId();

    IRpcClientObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL ||
            m_msgClient == NULL)
        {
            return;
        }

        if (msgClient != m_msgClient)
        {
            return;
        }

        m_clientId = clientId;

        m_observer->AddRef();
        observer = m_observer;
    }

    if (0)
    {{{
        char suiteName[64] = "";
        msgClient->GetSslSuite(suiteName);

        CProStlString timeString = "";
        ProGetLocalTimeString(timeString);

        printf(
            "\n"
            "%s \n"
            " CRpcClient::OnOkMsg(id : " PRO_PRT64U ", publicIp : %s,"
            " sslSuite : %s, server : %s:%u, mmType : %u) \n"
            ,
            timeString.c_str(),
            clientId,
            myPublicIp,
            suiteName,
            m_msgConfigInfo.msgc_server_ip.c_str(),
            (unsigned int)m_msgConfigInfo.msgc_server_port,
            (unsigned int)m_msgConfigInfo.msgc_mm_type
            );
    }}}

    observer->OnLogon(this, clientId, myPublicIp);
    observer->Release();
}

void
CRpcClient::OnRecvMsg(IRtpMsgClient*      msgClient,
                      const void*         buf,
                      unsigned long       size,
                      PRO_UINT16          charset,
                      const RTP_MSG_USER* srcUser)
{
    assert(msgClient != NULL);
    assert(buf != NULL);
    assert(size > 0);
    assert(srcUser != NULL);
    if (msgClient == NULL || buf == NULL || size == 0 || srcUser == NULL)
    {
        return;
    }

    if (srcUser->classId  != RPC_ROOT_ID.classId ||
        srcUser->UserId() != RPC_ROOT_ID.UserId())
    {
        RecvMsg(msgClient, buf, size, charset, srcUser->UserId());

        return;
    }

    RPC_HDR                     hdr;
    CProStlVector<RPC_ARGUMENT> args;

    if (CRpcPacket::ParseRpcPacket(buf, size, hdr, args))
    {
        RecvRpc(msgClient, hdr, args);
    }
    else
    {
        RecvMsg(msgClient, buf, size, charset, 0);
    }
}

void
CRpcClient::RecvRpc(IRtpMsgClient*                     msgClient,
                    RPC_HDR                            hdr,
                    const CProStlVector<RPC_ARGUMENT>& args)
{
    assert(msgClient != NULL);

    if (hdr.requestId == 0 || hdr.functionId == 0)
    {
        return;
    }

    IRpcClientObserver* observer = NULL;
    CRpcPacket*         result   = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL ||
            m_msgClient == NULL)
        {
            return;
        }

        if (msgClient != m_msgClient)
        {
            return;
        }

        {
            CProStlMap<PRO_UINT32, RPC_FUNCTION_INFO>::iterator const itr =
                m_funtionId2Info.find(hdr.functionId);
            if (itr == m_funtionId2Info.end())
            {
                return;
            }

            const RPC_FUNCTION_INFO& info = itr->second;
            if (!CmpRpcArgsTypes(args, info.retnArgTypes))
            {
                return;
            }
        }

        CProStlMap<PRO_UINT64, PRO_UINT64>::iterator const itr =
            m_requestId2TimerId.find(hdr.requestId);
        if (itr == m_requestId2TimerId.end())
        {
            return;
        }

        const RPC_HDR2 hdr2 = m_timerId2Hdr[itr->second];

        m_reactor->CancelTimer(itr->second);
        m_timerId2Hdr.erase(itr->second);
        m_requestId2TimerId.erase(itr);

        if (hdr.rpcCode == RPCE_OK)
        {
            result = CRpcPacket::CreateInstance(
                hdr.requestId,
                hdr.functionId,
                true /* this is a rebuilt packet */
                );
            if (result == NULL)
            {
                hdr.rpcCode = RPCE_NOT_ENOUGH_MEMORY;
            }
        }

        if (result != NULL)
        {
            assert(hdr.rpcCode == RPCE_OK);

            result->SetClientId(m_clientId);
            result->SetRpcCode(hdr.rpcCode);
            result->SetMagic1(hdr2.magic1);
            result->SetMagic2(hdr2.magic2);

            if (args.size() > 0)
            {
                result->CleanAndBeginPushArgument();
                if (!result->PushArguments(&args[0], args.size()) ||
                    !result->EndPushArgument())
                {
                    hdr.rpcCode = RPCE_NOT_ENOUGH_MEMORY;
                    result->Release();
                    result = NULL;
                }
            }
            else
            {
                result->CleanAndBeginPushArgument();
                if (!result->EndPushArgument())
                {
                    hdr.rpcCode = RPCE_NOT_ENOUGH_MEMORY;
                    result->Release();
                    result = NULL;
                }
            }
        }

        if (result == NULL)
        {
            assert(hdr.rpcCode != RPCE_OK);

            m_packet->AddRef();
            result = m_packet;

            result->SetClientId(m_clientId);
            result->SetRequestId(hdr.requestId);
            result->SetFunctionId(hdr.functionId);
            result->SetRpcCode(hdr.rpcCode);
            result->SetMagic1(hdr2.magic1);
            result->SetMagic2(hdr2.magic2);
        }

        m_observer->AddRef();
        observer = m_observer;
    }

    observer->OnRpcResult(this, result);
    observer->Release();
    result->Release();
}

void
CRpcClient::RecvMsg(IRtpMsgClient* msgClient,
                    const void*    buf,
                    unsigned long  size,
                    PRO_UINT16     charset,
                    PRO_UINT64     srcClientId)
{
    assert(msgClient != NULL);
    assert(buf != NULL);
    assert(size > 0);

    IRpcClientObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL ||
            m_msgClient == NULL)
        {
            return;
        }

        if (msgClient != m_msgClient)
        {
            return;
        }

        m_observer->AddRef();
        observer = m_observer;
    }

    if (srcClientId == 0)
    {
        observer->OnRecvMsgFromServer(this, buf, size, charset);
    }
    else
    {
        observer->OnRecvMsgFromClient(this, buf, size, charset, srcClientId);
    }

    observer->Release();
}

void
CRpcClient::OnCloseMsg(IRtpMsgClient* msgClient,
                       long           errorCode,
                       long           sslCode,
                       bool           tcpConnected)
{
    assert(msgClient != NULL);
    if (msgClient == NULL)
    {
        return;
    }

    IRpcClientObserver*              observer = NULL;
    CRpcPacket*                      result   = NULL;
    PRO_UINT64                       clientId = 0;
    CProStlMap<PRO_UINT64, RPC_HDR2> timerId2Hdr;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL ||
            m_msgClient == NULL)
        {
            return;
        }

        if (msgClient != m_msgClient)
        {
            return;
        }

        m_requestId2TimerId.clear();
        timerId2Hdr = m_timerId2Hdr;
        m_timerId2Hdr.clear();
        clientId = m_clientId;
        m_clientId = 0;

        m_observer->AddRef();
        m_packet->AddRef();
        observer = m_observer;
        result   = m_packet;
    }

    if (0)
    {{{
        CProStlString timeString = "";
        ProGetLocalTimeString(timeString);

        printf(
            "\n"
            "%s \n"
            " CRpcClient::OnCloseMsg(id : " PRO_PRT64U ","
            " errorCode : [%d, %d], tcpConnected : %d, server : %s:%u,"
            " mmType : %u) \n"
            ,
            timeString.c_str(),
            clientId,
            (int)errorCode,
            (int)sslCode,
            (int)tcpConnected,
            m_msgConfigInfo.msgc_server_ip.c_str(),
            (unsigned int)m_msgConfigInfo.msgc_server_port,
            (unsigned int)m_msgConfigInfo.msgc_mm_type
            );
    }}}

    CProStlMap<PRO_UINT64, RPC_HDR2>::iterator       itr = timerId2Hdr.begin();
    CProStlMap<PRO_UINT64, RPC_HDR2>::iterator const end = timerId2Hdr.end();

    for (; itr != end; ++itr)
    {
        const RPC_HDR2& hdr = itr->second;

        result->SetClientId(clientId);
        result->SetRequestId(hdr.requestId);
        result->SetFunctionId(hdr.functionId);
        result->SetRpcCode(RPCE_NETWORK_BROKEN);
        result->SetMagic1(hdr.magic1);
        result->SetMagic2(hdr.magic2);

        observer->OnRpcResult(this, result);
    }

    observer->OnLogoff(this, errorCode, sslCode, tcpConnected);
    observer->Release();
    result->Release();
}

void
CRpcClient::OnTimer(void*      factory,
                    PRO_UINT64 timerId,
                    PRO_INT64  userData)
{
    assert(factory != NULL);
    assert(timerId > 0);
    if (factory == NULL || timerId == 0)
    {
        return;
    }

    IRpcClientObserver* observer = NULL;
    CRpcPacket*         result   = NULL;
    PRO_UINT64          clientId = 0;
    RPC_HDR2            hdr;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_reactor == NULL || m_packet == NULL)
        {
            return;
        }

        CProStlMap<PRO_UINT64, RPC_HDR2>::iterator const itr =
            m_timerId2Hdr.find(timerId);
        if (itr == m_timerId2Hdr.end())
        {
            return;
        }

        hdr = itr->second;

        m_reactor->CancelTimer(timerId);
        m_requestId2TimerId.erase(hdr.requestId);
        m_timerId2Hdr.erase(itr);

        m_observer->AddRef();
        m_packet->AddRef();
        observer = m_observer;
        result   = m_packet;
        clientId = m_clientId;
    }

    result->SetClientId(clientId);
    result->SetRequestId(hdr.requestId);
    result->SetFunctionId(hdr.functionId);
    result->SetRpcCode(RPCE_NETWORK_TIMEOUT);
    result->SetMagic1(hdr.magic1);
    result->SetMagic2(hdr.magic2);

    observer->OnRpcResult(this, result);
    observer->Release();
    result->Release();
}
