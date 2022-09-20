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
        magic1 = 0;
        magic2 = 0;
    }

    PRO_INT64 magic1;
    PRO_INT64 magic2;

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

    static CRpcClient* CreateInstance();

    bool Init(
        IRpcClientObserver* observer,
        IProReactor*        reactor,
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

    virtual PRO_UINT64 GetClientId() const;

    virtual const char* GetServerIp(char serverIp[64]) const;

    virtual unsigned short GetServerPort() const;

    virtual const char* GetLocalIp(char localIp[64]) const;

    virtual unsigned short GetLocalPort() const;

    virtual RPC_ERROR_CODE RegisterFunction(
        PRO_UINT32           functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        );

    virtual void UnregisterFunction(PRO_UINT32 functionId);

    virtual RPC_ERROR_CODE SendRpcRequest(
        IRpcPacket*   request,
        bool          noreply,            /* = false */
        unsigned long rpcTimeoutInSeconds /* = 0 */
        );

    virtual bool SendMsgToServer(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset
        );

    virtual bool SendMsgToClients(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset,
        const PRO_UINT64* dstClients,
        unsigned char     dstClientCount
        );

    virtual bool Reconnect();

    virtual void SetMagic(PRO_INT64 magic);

    virtual PRO_INT64 GetMagic() const;

private:

    CRpcClient();

    virtual ~CRpcClient();

    virtual void OnOkMsg(
        IRtpMsgClient*      msgClient,
        const RTP_MSG_USER* myUser,
        const char*         myPublicIp
        );

    virtual void OnRecvMsg(
        IRtpMsgClient*      msgClient,
        const void*         buf,
        unsigned long       size,
        PRO_UINT16          charset,
        const RTP_MSG_USER* srcUser
        );

    virtual void OnCloseMsg(
        IRtpMsgClient* msgClient,
        long           errorCode,
        long           sslCode,
        bool           tcpConnected
        );

    virtual void OnTimer(
        void*      factory,
        PRO_UINT64 timerId,
        PRO_INT64  userData
        );

    void RecvRpc(
        IRtpMsgClient*                     msgClient,
        RPC_HDR                            hdr,
        const CProStlVector<RPC_ARGUMENT>& args
        );

    void RecvMsg(
        IRtpMsgClient* msgClient,
        const void*    buf,
        unsigned long  size,
        PRO_UINT16     charset,
        PRO_UINT64     srcClientId
        );

private:

    IRpcClientObserver*                       m_observer;
    RPC_CLIENT_CONFIG_INFO                    m_configInfo;
    CRpcPacket*                               m_packet;
    PRO_UINT64                                m_clientId;
    PRO_INT64                                 m_magic;

    CProStlMap<PRO_UINT32, RPC_FUNCTION_INFO> m_funtionId2Info;
    CProStlMap<PRO_UINT64, RPC_HDR2>          m_timerId2Hdr;
    CProStlMap<PRO_UINT64, PRO_UINT64>        m_requestId2TimerId;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_CLIENT_H */
