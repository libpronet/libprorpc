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

#if !defined(RPC_CLIENT_H)
#define RPC_CLIENT_H

#include "pro_rpc.h"
#include "rpc_packet.h"
#include "rpc_server.h"
#include "promsg/msg_client.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_timer_factory.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"

/////////////////////////////////////////////////////////////////////////////
////

struct RPC_CLIENT_CONFIG_INFO
{
    RPC_CLIENT_CONFIG_INFO()
    {
        rpcc_pending_calls = 10000;
        rpcc_rpc_timeout   = 10;
    }

    unsigned int rpcc_pending_calls;
    unsigned int rpcc_rpc_timeout; /* 1 ~ 3600 */

    DECLARE_SGI_POOL(0)
};

struct RPC_HDR2 : public RPC_HDR
{
    RPC_HDR2()
    {
        memset(this, 0, sizeof(RPC_HDR));

        magic1 = 0;
        magic2 = 0;
    }

    int64_t       magic1;
    int64_t       magic2;
    CProStlString magicStr;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

class CRpcClient
:
public IRpcClient,
public IProOnTimer,
public CMsgClient
{
public:

    static CRpcClient* CreateInstance(
        int64_t magic, /* = 0 */
        int64_t magic2 /* = 0 */
        );

    bool Init(
        IRpcClientObserver* observer,
        IProReactor*        reactor,
        const char*         argv0,      /* = NULL */
        const char*         configFileName,
        RTP_MM_TYPE         mmType,     /* = 0 */
        const char*         serverIp,   /* = NULL */
        unsigned short      serverPort, /* = 0 */
        const RTP_MSG_USER* user,       /* = NULL */
        const char*         password,   /* = NULL */
        const char*         localIp     /* = NULL */
        );

    void Fini();

    virtual unsigned long AddRef();

    virtual unsigned long Release();

    virtual RTP_MM_TYPE GetMmType() const;

    virtual uint64_t GetClientId() const;

    virtual const char* GetServerIp(char serverIp[64]) const;

    virtual unsigned short GetServerPort() const;

    virtual const char* GetLocalIp(char localIp[64]) const;

    virtual unsigned short GetLocalPort() const;

    virtual RPC_ERROR_CODE RegisterFunction(
        uint32_t             functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        );

    virtual void UnregisterFunction(uint32_t functionId);

    virtual RPC_ERROR_CODE SendRpcRequest(
        IRpcPacket*  request,
        bool         noreply,            /* = false */
        unsigned int rpcTimeoutInSeconds /* = 0 */
        );

    virtual bool SendMsgToServer(
        const void* buf,
        size_t      size,
        uint16_t    charset
        );

    virtual bool SendMsgToClients(
        const void*     buf,
        size_t          size,
        uint16_t        charset,
        const uint64_t* dstClients,
        unsigned char   dstClientCount
        );

    virtual bool Reconnect();

    virtual void SetMagic(int64_t magic);

    virtual int64_t GetMagic() const;

    virtual void SetMagic2(int64_t magic2);

    virtual int64_t GetMagic2() const;

private:

    CRpcClient(
        int64_t magic, /* = 0 */
        int64_t magic2 /* = 0 */
        );

    virtual ~CRpcClient();

    virtual void OnOkMsg(
        IRtpMsgClient*      msgClient,
        const RTP_MSG_USER* myUser,
        const char*         myPublicIp
        );

    virtual void OnRecvMsg(
        IRtpMsgClient*      msgClient,
        const void*         buf,
        size_t              size,
        uint16_t            charset,
        const RTP_MSG_USER* srcUser
        );

    virtual void OnCloseMsg(
        IRtpMsgClient* msgClient,
        int            errorCode,
        int            sslCode,
        bool           tcpConnected
        );

    virtual void OnTimer(
        void*    factory,
        uint64_t timerId,
        int64_t  tick,
        int64_t  userData
        );

    void RecvRpc(
        IRtpMsgClient*                     msgClient,
        RPC_HDR                            hdr,
        const CProStlVector<RPC_ARGUMENT>& args
        );

    void RecvMsg(
        IRtpMsgClient* msgClient,
        const void*   buf,
        size_t        size,
        uint16_t      charset,
        uint64_t      srcClientId
        );

private:

    IRpcClientObserver*                     m_observer;
    RPC_CLIENT_CONFIG_INFO                  m_configInfo;
    CRpcPacket*                             m_packet;
    uint64_t                                m_clientId;
    int64_t                                 m_magic;
    int64_t                                 m_magic2;

    CProStlMap<uint32_t, RPC_FUNCTION_INFO> m_funtionId2Info;
    CProStlMap<uint64_t, RPC_HDR2>          m_timerId2Hdr;
    CProStlMap<uint64_t, uint64_t>          m_requestId2TimerId;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_CLIENT_H */
