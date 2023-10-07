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

#if !defined(TEST_H)
#define TEST_H

#include "pronet/pro_memory_pool.h"
#include "pronet/pro_ref_count.h"
#include "pronet/pro_stat.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include "../pro_rpc/pro_rpc.h"

/////////////////////////////////////////////////////////////////////////////
////

class CTest : public IRpcClientObserver, public CProRefCount
{
public:

    static CTest* CreateInstance();

    bool Init(IProReactor* reactor);

    void Fini();

    virtual unsigned long AddRef();

    virtual unsigned long Release();

    RTP_MM_TYPE GetMmType() const;

    unsigned short GetServerAddr(char serverIp[64]) const;

    void Test();

private:

    CTest();

    virtual ~CTest();

    virtual void OnLogon(
        IRpcClient* client,
        uint64_t    myClientId,
        const char* myPublicIp
        )
    {
    }

    virtual void OnLogoff(
        IRpcClient* client,
        int         errorCode,
        int         sslCode,
        bool        tcpConnected
        );

    virtual void OnRpcResult(
        IRpcClient* client,
        IRpcPacket* result
        );

    virtual void OnRecvMsgFromServer(
        IRpcClient* client,
        const void* buf,
        size_t      size,
        uint16_t    charset
        )
    {
    }

    virtual void OnRecvMsgFromClient(
        IRpcClient* client,
        const void* buf,
        size_t      size,
        uint16_t    charset,
        uint64_t    srcClientId
        )
    {
    }

    static void RegisterFunctions(IRpcClient* client);

    void Test1(int64_t tick);

    void Test2(
        int32_t       a,
        int32_t       b,
        const int32_t c[2],
        int64_t       tick
        );

    void Test1_ret(
        IRpcClient* client,
        IRpcPacket* result
        );

    void Test1_err(
        IRpcClient* client,
        IRpcPacket* result
        )
    {
    }

    void Test2_ret(
        IRpcClient* client,
        IRpcPacket* result
        );

    void Test2_err(
        IRpcClient* client,
        IRpcPacket* result
        )
    {
    }

private:

    IProReactor*            m_reactor;
    IRpcClient*             m_client;
    CProStatBitRate         m_stat;
    mutable CProThreadMutex m_lock;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* TEST_H */
