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

#include "rpc_server.h"
#include "pro_rpc.h"
#include "rpc_packet.h"
#include "promsg/msg_server.h"
#include "pronet/pro_channel_task_pool.h"
#include "pronet/pro_config_file.h"
#include "pronet/pro_functor_command.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_time_util.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
////

static const unsigned char C2S_CID = 1;
static const unsigned char RPC_CID = 2;
static const PRO_UINT16    RPC_IID = 1;

typedef void (CRpcServer::* ACTION)(PRO_INT64*);

/////////////////////////////////////////////////////////////////////////////
////

static
void
PRO_CALLTYPE
ReadConfig_i(const CProStlVector<PRO_CONFIG_ITEM>& configs,
             RPC_SERVER_CONFIG_INFO&               configInfo)
{
    int       i = 0;
    const int c = (int)configs.size();

    for (; i < c; ++i)
    {
        const CProStlString& configName  = configs[i].configName;
        const CProStlString& configValue = configs[i].configValue;

        if (stricmp(configName.c_str(), "rpcs_pending_calls") == 0)
        {
            const int value = atoi(configValue.c_str());
            if (value > 0)
            {
                configInfo.rpcs_pending_calls = value;
            }
        }
        else if (stricmp(configName.c_str(), "rpcs_worker_count") == 0)
        {
            const int value = atoi(configValue.c_str());
            if (value > 0 && value <= 100)
            {
                configInfo.rpcs_worker_count = value;
            }
        }
        else
        {
        }
    } /* end of for (...) */
}

/////////////////////////////////////////////////////////////////////////////
////

CRpcServer*
CRpcServer::CreateInstance()
{
    CRpcServer* const server = new CRpcServer;

    return (server);
}

CRpcServer::CRpcServer()
{
    m_observer = NULL;
    m_taskPool = NULL;
}

CRpcServer::~CRpcServer()
{
    Fini();
}

bool
CRpcServer::Init(IRpcServerObserver* observer,
                 IProReactor*        reactor,
                 const char*         configFileName,
                 RTP_MM_TYPE         mmType,         /* = 0 */
                 unsigned short      serviceHubPort) /* = 0 */
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

    RPC_SERVER_CONFIG_INFO configInfo;
    ReadConfig_i(configs, configInfo);

    CProChannelTaskPool* taskPool = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        assert(m_observer == NULL);
        assert(m_taskPool == NULL);
        if (m_observer != NULL || m_taskPool != NULL)
        {
            return (false);
        }

        if (!CMsgServer::Init(
            reactor, configFileName, mmType, serviceHubPort))
        {
            goto EXIT;
        }

        taskPool = new CProChannelTaskPool;
        if (!taskPool->Start(configInfo.rpcs_worker_count))
        {
            goto EXIT;
        }

        observer->AddRef();
        m_observer   = observer;
        m_configInfo = configInfo;
        m_taskPool   = taskPool;
    }

    return (true);

EXIT:

    if (taskPool != NULL)
    {
        taskPool->Stop();
        delete taskPool;
    }

    CMsgServer::Fini();

    return (false);
}

void
CRpcServer::Fini()
{
    IRpcServerObserver*  observer = NULL;
    CProChannelTaskPool* taskPool = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL)
        {
            return;
        }

        m_funtionId2Info.clear();
        taskPool = m_taskPool;
        m_taskPool = NULL;
        observer = m_observer;
        m_observer = NULL;
    }

    taskPool->Stop();
    delete taskPool;
    observer->Release();

    CMsgServer::Fini();
}

unsigned long
PRO_CALLTYPE
CRpcServer::AddRef()
{
    const unsigned long refCount = CMsgServer::AddRef();

    return (refCount);
}

unsigned long
PRO_CALLTYPE
CRpcServer::Release()
{
    const unsigned long refCount = CMsgServer::Release();

    return (refCount);
}

RTP_MM_TYPE
PRO_CALLTYPE
CRpcServer::GetMmType() const
{
    RTP_MM_TYPE mmType = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        mmType = m_msgConfigInfo.msgs_mm_type;
    }

    return (mmType);
}

unsigned short
PRO_CALLTYPE
CRpcServer::GetServicePort() const
{
    unsigned short servicePort = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        servicePort = m_msgConfigInfo.msgs_hub_port;
    }

    return (servicePort);
}

RPC_ERROR_CODE
PRO_CALLTYPE
CRpcServer::RegisterFunction(PRO_UINT32           functionId,
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

        if (m_observer == NULL || m_taskPool == NULL)
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
PRO_CALLTYPE
CRpcServer::UnregisterFunction(PRO_UINT32 functionId)
{
    if (functionId == 0)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL)
        {
            return;
        }

        m_funtionId2Info.erase(functionId);
    }
}

RPC_ERROR_CODE
PRO_CALLTYPE
CRpcServer::SendRpcResult(IRpcPacket* result)
{
    assert(result != NULL);
    if (result == NULL)
    {
        return (RPCE_INVALID_ARGUMENT);
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return (RPCE_ERROR);
        }

        CProStlMap<PRO_UINT32, RPC_FUNCTION_INFO>::iterator const itr =
            m_funtionId2Info.find(result->GetFunctionId());
        if (itr == m_funtionId2Info.end())
        {
            return (RPCE_INVALID_FUNCTION);
        }

        if (result->GetRpcCode() == RPCE_OK)
        {
            const RPC_FUNCTION_INFO& info = itr->second;
            if (!CmpRpcPacketTypes(result, info.retnArgTypes))
            {
                return (RPCE_MISMATCHED_PARAMETER);
            }
        }

        const RTP_MSG_USER user(RPC_CID, result->GetClientId(), RPC_IID);

        if (!m_msgServer->SendMsg(
            result->GetTotalBuffer(), result->GetTotalSize(), 0, &user, 1))
        {
            return (RPCE_ERROR);
        }
    }

    return (RPCE_OK);
}

bool
PRO_CALLTYPE
CRpcServer::SendMsgToClients(const void*       buf,
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

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return (false);
        }

        ret = m_msgServer->SendMsg(buf, size, charset,
            &dstUsers[0], (unsigned char)dstUsers.size());
    }

    return (ret);
}

void
PRO_CALLTYPE
CRpcServer::KickoutClient(PRO_UINT64 clientId)
{
    if (clientId == 0)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return;
        }

        const RTP_MSG_USER user(RPC_CID, clientId, RPC_IID);
        m_msgServer->KickoutUser(&user);
    }
}

bool
PRO_CALLTYPE
CRpcServer::OnCheckUser(IRtpMsgServer*      msgServer,
                        const RTP_MSG_USER* user,
                        const char*         userPublicIp,
                        const RTP_MSG_USER* c2sUser, /* = NULL */
                        const char          hash[32],
                        const char          nonce[32],
                        PRO_UINT64*         userId,
                        PRO_UINT16*         instId,
                        PRO_INT64*          appData,
                        bool*               isC2s)
{
    if (!CMsgServer::OnCheckUser(msgServer, user, userPublicIp,
        c2sUser, hash, nonce, userId, instId, appData, isC2s))
    {
        return (false);
    }

    if (user->classId == C2S_CID)
    {
        *isC2s  = true;
    }
    else if (user->classId == RPC_CID)
    {
        *instId = RPC_IID;
    }
    else
    {
        return (false);
    }

    return (true);
}

void
PRO_CALLTYPE
CRpcServer::OnOkUser(IRtpMsgServer*      msgServer,
                     const RTP_MSG_USER* user,
                     const char*         userPublicIp,
                     const RTP_MSG_USER* c2sUser, /* = NULL */
                     PRO_INT64           appData)
{
    assert(msgServer != NULL);
    assert(user != NULL);
    assert(userPublicIp != NULL);
    assert(userPublicIp[0] != '\0');
    if (msgServer == NULL || user == NULL || userPublicIp == NULL ||
        userPublicIp[0] == '\0')
    {
        return;
    }

    if (user->classId != RPC_CID || user->instId != RPC_IID)
    {
        return;
    }

    const PRO_UINT64 clientId = user->UserId();

    IRpcServerObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return;
        }

        if (msgServer != m_msgServer)
        {
            return;
        }

        m_taskPool->AddChannel(clientId);

        m_observer->AddRef();
        observer = m_observer;
    }

    observer->OnLogon(this, clientId, userPublicIp);
    observer->Release();
}

void
PRO_CALLTYPE
CRpcServer::OnCloseUser(IRtpMsgServer*      msgServer,
                        const RTP_MSG_USER* user,
                        long                errorCode,
                        long                sslCode)
{
    assert(msgServer != NULL);
    assert(user != NULL);
    if (msgServer == NULL || user == NULL)
    {
        return;
    }

    if (user->classId != RPC_CID || user->instId != RPC_IID)
    {
        return;
    }

    const PRO_UINT64 clientId = user->UserId();

    IRpcServerObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return;
        }

        if (msgServer != m_msgServer)
        {
            return;
        }

        m_taskPool->RemoveChannel(clientId);

        m_observer->AddRef();
        observer = m_observer;
    }

    observer->OnLogoff(this, clientId, errorCode, sslCode);
    observer->Release();
}

void
PRO_CALLTYPE
CRpcServer::OnRecvMsg(IRtpMsgServer*      msgServer,
                      const void*         buf,
                      unsigned long       size,
                      PRO_UINT16          charset,
                      const RTP_MSG_USER* srcUser)
{
    assert(msgServer != NULL);
    assert(buf != NULL);
    assert(size > 0);
    assert(srcUser != NULL);
    if (msgServer == NULL || buf == NULL || size == 0 || srcUser == NULL)
    {
        return;
    }

    if (srcUser->classId != RPC_CID || srcUser->instId != RPC_IID)
    {
        return;
    }

    const PRO_UINT64 srcClientId = srcUser->UserId();

    RPC_HDR                     hdr;
    CProStlVector<RPC_ARGUMENT> args;

    if (CRpcPacket::ParseRpcPacket(buf, size, hdr, args))
    {
        RecvRpc(msgServer, hdr, args, srcClientId);
    }
    else
    {
        RecvMsg(msgServer, buf, size, charset, srcClientId);
    }
}

void
CRpcServer::RecvRpc(IRtpMsgServer*                     msgServer,
                    RPC_HDR                            hdr,
                    const CProStlVector<RPC_ARGUMENT>& args,
                    PRO_UINT64                         srcClientId)
{
    assert(msgServer != NULL);

    if (srcClientId == 0 || hdr.requestId == 0 || hdr.functionId == 0)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return;
        }

        if (msgServer != m_msgServer)
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
            if (!CmpRpcArgsTypes(args, info.callArgTypes))
            {
                return;
            }
        }

        if (m_taskPool->GetSize() >= m_configInfo.rpcs_pending_calls)
        {
            if (!hdr.noreply)
            {
                SendErrorCode(srcClientId, hdr.requestId, hdr.functionId,
                    RPCE_SERVER_BUSY);
            }

            return;
        }

        CRpcPacket* request = CRpcPacket::CreateInstance(
            hdr.requestId,
            hdr.functionId,
            true /* this is a rebuilt packet */
            );
        if (request == NULL)
        {
            return;
        }

        request->SetClientId(srcClientId);
        request->SetNoreply(hdr.noreply);
        request->SetTimeout(hdr.timeoutInSeconds);

        if (args.size() > 0)
        {
            request->CleanAndBeginPushArgument();
            if (!request->PushArguments(&args[0], args.size()) ||
                !request->EndPushArgument())
            {
                request->Release();
                request = NULL;
            }
        }
        else
        {
            request->CleanAndBeginPushArgument();
            if (!request->EndPushArgument())
            {
                request->Release();
                request = NULL;
            }
        }

        if (request == NULL)
        {
            return;
        }

        IProFunctorCommand* const command =
            CProFunctorCommand_cpp<CRpcServer, ACTION>::CreateInstance(
            *this,
            &CRpcServer::AsyncRecvRpc,
            (PRO_INT64)request,
            (PRO_INT64)ProGetTickCount64() /* arrival time */
            );
        m_taskPool->Put(srcClientId, command);
    }
}

void
CRpcServer::AsyncRecvRpc(PRO_INT64* args)
{
    CRpcPacket* const request     = (CRpcPacket*)args[0];
    const PRO_INT64   arrivalTick =              args[1];

    /*
     * check timeout
     */
    if (ProGetTickCount64() >=
        arrivalTick + (PRO_INT64)request->GetTimeout() * 1000)
    {
        request->Release();

        return;
    }

    IRpcServerObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer != NULL)
        {
            m_observer->AddRef();
            observer = m_observer;
        }
    }

    if (observer != NULL)
    {
        observer->OnRpcRequest(this, request);
        observer->Release();
    }

    request->Release();
}

void
CRpcServer::RecvMsg(IRtpMsgServer*      msgServer,
                    const void*         buf,
                    unsigned long       size,
                    PRO_UINT16          charset,
                    PRO_UINT64          srcClientId)
{
    assert(msgServer != NULL);
    assert(buf != NULL);
    assert(size > 0);

    IRpcServerObserver* observer = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_observer == NULL || m_taskPool == NULL || m_msgServer == NULL)
        {
            return;
        }

        if (msgServer != m_msgServer)
        {
            return;
        }

        m_observer->AddRef();
        observer = m_observer;
    }

    observer->OnRecvMsg(this, buf, size, charset, srcClientId);
    observer->Release();
}

void
CRpcServer::SendErrorCode(PRO_UINT64     clientId,
                          PRO_UINT64     requestId,
                          PRO_UINT32     functionId,
                          RPC_ERROR_CODE rpcCode)
{
    assert(clientId > 0);
    assert(requestId > 0);
    assert(functionId > 0);
    assert(rpcCode < 0);
    assert(m_msgServer != NULL);

    IRpcPacket* const result =
        CreateRpcResult(clientId, requestId, functionId, rpcCode, NULL, 0);
    if (result == NULL)
    {
        return;
    }

    const RTP_MSG_USER user(RPC_CID, clientId, RPC_IID);

    m_msgServer->SendMsg(
        result->GetTotalBuffer(), result->GetTotalSize(), 0, &user, 1);
    result->Release();
}
