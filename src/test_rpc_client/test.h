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

    virtual unsigned long PRO_CALLTYPE AddRef();

    virtual unsigned long PRO_CALLTYPE Release();

    RTP_MM_TYPE GetMmType() const;

    unsigned short GetServerAddr(char serverIp[64]) const;

    void Test();

private:

    CTest();

    virtual ~CTest();

    virtual void PRO_CALLTYPE OnLogon(
        IRpcClient* client,
        PRO_UINT64  myClientId,
        const char* myPublicIp
        )
    {
    }

    virtual void PRO_CALLTYPE OnLogoff(
        IRpcClient* client,
        long        errorCode,
        long        sslCode,
        bool        tcpConnected
        );

    virtual void PRO_CALLTYPE OnRpcResult(
        IRpcClient* client,
        IRpcPacket* result
        );

    virtual void PRO_CALLTYPE OnRecvMsg(
        IRpcClient*   client,
        const void*   buf,
        unsigned long size,
        PRO_UINT16    charset,
        PRO_UINT64    srcClientId
        )
    {
    }

    static void RegisterFunctions(IRpcClient* client);

    void Test1(PRO_INT64 tick);

    void Test2(
        PRO_INT32       a,
        PRO_INT32       b,
        const PRO_INT32 c[2],
        PRO_INT64       tick
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

    DECLARE_SGI_POOL(0);
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* TEST_H */
