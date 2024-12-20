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

#if !defined(RPC_SERVER_H)
#define RPC_SERVER_H

#include "pro_rpc.h"
#include "rpc_packet.h"
#include "promsg/msg_server.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"

/////////////////////////////////////////////////////////////////////////////
////

class CProChannelTaskPool;

struct RPC_SERVER_CONFIG_INFO
{
    RPC_SERVER_CONFIG_INFO()
    {
        rpcs_pending_calls = 10000;
        rpcs_worker_count  = 2;
    }

    unsigned int rpcs_pending_calls;
    unsigned int rpcs_worker_count; /* 1 ~ 100 */

    DECLARE_SGI_POOL(0)
};

struct RPC_FUNCTION_INFO
{
    CProStlVector<RPC_DATA_TYPE> callArgTypes;
    CProStlVector<RPC_DATA_TYPE> retnArgTypes;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

class CRpcServer : public IRpcServer, public CMsgServer
{
public:

    static CRpcServer* CreateInstance();

    bool Init(
        IRpcServerObserver* observer,
        IProReactor*        reactor,
        const char*         argv0,         /* = NULL */
        const char*         configFileName,
        RTP_MM_TYPE         mmType,        /* = 0 */
        unsigned short      serviceHubPort /* = 0 */
        );

    void Fini();

    virtual unsigned long AddRef();

    virtual unsigned long Release();

    virtual RTP_MM_TYPE GetMmType() const;

    virtual unsigned short GetServicePort() const;

    virtual RPC_ERROR_CODE RegisterFunction(
        uint32_t             functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        );

    virtual void UnregisterFunction(uint32_t functionId);

    virtual RPC_ERROR_CODE SendRpcResult(IRpcPacket* result);

    virtual bool SendMsgToClients(
        const void*     buf,
        size_t          size,
        uint16_t        charset,
        const uint64_t* dstClients,
        unsigned char   dstClientCount
        );

    virtual void KickoutClient(uint64_t clientId);

private:

    CRpcServer();

    virtual ~CRpcServer();

    virtual bool OnCheckUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        const char*         userPublicIp,
        const RTP_MSG_USER* c2sUser, /* = NULL */
        const unsigned char hash[32],
        const unsigned char nonce[32],
        uint64_t*           userId,
        uint16_t*           instId,
        int64_t*            appData,
        bool*               isC2s
        );

    virtual void OnOkUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        const char*         userPublicIp,
        const RTP_MSG_USER* c2sUser, /* = NULL */
        int64_t             appData
        );

    virtual void OnCloseUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        int                 errorCode,
        int                 sslCode
        );

    virtual void OnRecvMsg(
        IRtpMsgServer*      msgServer,
        const void*         buf,
        size_t              size,
        uint16_t            charset,
        const RTP_MSG_USER* srcUser
        );

    void RecvRpc(
        IRtpMsgServer*                     msgServer,
        RPC_HDR                            hdr,
        const CProStlVector<RPC_ARGUMENT>& args,
        uint64_t                           srcClientId
        );

    void RecvMsg(
        IRtpMsgServer* msgServer,
        const void*    buf,
        size_t         size,
        uint16_t       charset,
        uint64_t       srcClientId
        );

    void SendErrorCode(
        uint64_t       clientId,
        uint64_t       requestId,
        uint32_t       functionId,
        RPC_ERROR_CODE rpcCode
        );

    void AsyncRecvRpc(int64_t* args);

private:

    IRpcServerObserver*                     m_observer;
    RPC_SERVER_CONFIG_INFO                  m_configInfo;
    CProChannelTaskPool*                    m_taskPool;
    CProStlMap<uint32_t, RPC_FUNCTION_INFO> m_funtionId2Info;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_SERVER_H */
