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
        const char*         configFileName,
        RTP_MM_TYPE         mmType,        /* = 0 */
        unsigned short      serviceHubPort /* = 0 */
        );

    void Fini();

    virtual unsigned long PRO_CALLTYPE AddRef();

    virtual unsigned long PRO_CALLTYPE Release();

    virtual RTP_MM_TYPE PRO_CALLTYPE GetMmType() const;

    virtual unsigned short PRO_CALLTYPE GetServicePort() const;

    virtual RPC_ERROR_CODE PRO_CALLTYPE RegisterFunction(
        PRO_UINT32           functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        );

    virtual void PRO_CALLTYPE UnregisterFunction(PRO_UINT32 functionId);

    virtual RPC_ERROR_CODE PRO_CALLTYPE SendRpcResult(IRpcPacket* result);

    virtual bool PRO_CALLTYPE SendMsgToClients(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset,
        const PRO_UINT64* dstClients,
        unsigned char     dstClientCount
        );

    virtual void PRO_CALLTYPE KickoutClient(PRO_UINT64 clientId);

private:

    CRpcServer();

    virtual ~CRpcServer();

    virtual bool PRO_CALLTYPE OnCheckUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        const char*         userPublicIp,
        const RTP_MSG_USER* c2sUser, /* = NULL */
        const char          hash[32],
        const char          nonce[32],
        PRO_UINT64*         userId,
        PRO_UINT16*         instId,
        PRO_INT64*          appData,
        bool*               isC2s
        );

    virtual void PRO_CALLTYPE OnOkUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        const char*         userPublicIp,
        const RTP_MSG_USER* c2sUser, /* = NULL */
        PRO_INT64           appData
        );

    virtual void PRO_CALLTYPE OnCloseUser(
        IRtpMsgServer*      msgServer,
        const RTP_MSG_USER* user,
        long                errorCode,
        long                sslCode
        );

    virtual void PRO_CALLTYPE OnRecvMsg(
        IRtpMsgServer*      msgServer,
        const void*         buf,
        unsigned long       size,
        PRO_UINT16          charset,
        const RTP_MSG_USER* srcUser
        );

    void RecvRpc(
        IRtpMsgServer*                     msgServer,
        RPC_HDR                            hdr,
        const CProStlVector<RPC_ARGUMENT>& args,
        PRO_UINT64                         srcClientId
        );

    void RecvMsg(
        IRtpMsgServer* msgServer,
        const void*    buf,
        unsigned long  size,
        PRO_UINT16     charset,
        PRO_UINT64     srcClientId
        );

    void SendErrorCode(
        PRO_UINT64     clientId,
        PRO_UINT64     requestId,
        PRO_UINT32     functionId,
        RPC_ERROR_CODE rpcCode
        );

    void AsyncRecvRpc(PRO_INT64* args);

private:

    IRpcServerObserver*                       m_observer;
    RPC_SERVER_CONFIG_INFO                    m_configInfo;
    CProChannelTaskPool*                      m_taskPool;
    CProStlMap<PRO_UINT32, RPC_FUNCTION_INFO> m_funtionId2Info;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_SERVER_H */
