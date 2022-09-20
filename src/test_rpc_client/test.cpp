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

#include "test.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_ref_count.h"
#include "pronet/pro_stat.h"
#include "pronet/pro_thread_mutex.h"
#include "pronet/pro_time_util.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include "../pro_rpc/pro_rpc.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
////

#define RPC_FUNCTION_ID1 1
#define RPC_FUNCTION_ID2 2

/////////////////////////////////////////////////////////////////////////////
////

CTest*
CTest::CreateInstance()
{
    CTest* const tester = new CTest;

    return (tester);
}

CTest::CTest()
{
    m_reactor = NULL;
    m_client  = NULL;
}

CTest::~CTest()
{
    Fini();
}

bool
CTest::Init(IProReactor* reactor)
{
    assert(reactor != NULL);
    if (reactor == NULL)
    {
        return (false);
    }

    {
        CProThreadMutexGuard mon(m_lock);

        assert(m_reactor == NULL);
        assert(m_client == NULL);
        if (m_reactor != NULL || m_client != NULL)
        {
            return (false);
        }

        m_client = CreateRpcClient(
            this,
            reactor,
            "rpc_client.cfg",
            0,    /* mmType */
            NULL, /* serverIp */
            0,    /* serverPort */
            NULL, /* user */
            NULL, /* password */
            NULL  /* localIp */
            );
        if (m_client == NULL)
        {
            return (false);
        }

        RegisterFunctions(m_client);

        m_reactor = reactor;
    }

    return (true);
}

void
CTest::RegisterFunctions(IRpcClient* client)
{
    assert(client != NULL);

    /*
     * function1
     *
     * bool
     * ReturnTrue(PRO_INT64& tick);
     */
    {
        const RPC_DATA_TYPE callArgTypes[] =
        {
            RPC_DT_INT64
        };
        const RPC_DATA_TYPE retnArgTypes[] =
        {
            RPC_DT_BOOL8,
            RPC_DT_INT64
        };
        client->RegisterFunction(
            RPC_FUNCTION_ID1,
            callArgTypes,
            sizeof(callArgTypes) / sizeof(RPC_DATA_TYPE),
            retnArgTypes,
            sizeof(retnArgTypes) / sizeof(RPC_DATA_TYPE)
            );
    }

    /*
     * function2
     *
     * PRO_INT32
     * Sum(PRO_INT32 a, PRO_INT32 b, const PRO_INT32 c[2], PRO_INT64& tick);
     */
    {
        const RPC_DATA_TYPE callArgTypes[] =
        {
            RPC_DT_INT32,
            RPC_DT_INT32,
            RPC_DT_INT32ARRAY,
            RPC_DT_INT64
        };
        const RPC_DATA_TYPE retnArgTypes[] =
        {
            RPC_DT_INT32,
            RPC_DT_INT64
        };
        client->RegisterFunction(
            RPC_FUNCTION_ID2,
            callArgTypes,
            sizeof(callArgTypes) / sizeof(RPC_DATA_TYPE),
            retnArgTypes,
            sizeof(retnArgTypes) / sizeof(RPC_DATA_TYPE)
            );
    }
}

void
CTest::Fini()
{
    IRpcClient* client = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        client = m_client;
        m_client = NULL;
        m_reactor = NULL;
    }

    DeleteRpcClient(client);
}

unsigned long
CTest::AddRef()
{
    const unsigned long refCount = CProRefCount::AddRef();

    return (refCount);
}

unsigned long
CTest::Release()
{
    const unsigned long refCount = CProRefCount::Release();

    return (refCount);
}

RTP_MM_TYPE
CTest::GetMmType() const
{
    RTP_MM_TYPE mmType = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_client != NULL)
        {
            mmType = m_client->GetMmType();
        }
    }

    return (mmType);
}

unsigned short
CTest::GetServerAddr(char serverIp[64]) const
{
    strcpy(serverIp, "0.0.0.0");

    unsigned short serverPort = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_client != NULL)
        {
            m_client->GetServerIp(serverIp);
            serverPort = m_client->GetServerPort();
        }
    }

    return (serverPort);
}

void
CTest::Test()
{
    const PRO_INT64 tick = ProGetTickCount64();

    Test1(tick);

    const PRO_INT32 a    = 0;
    const PRO_INT32 b    = 1;
    const PRO_INT32 c[2] = { 2, 3 };
    Test2(a, b, c, tick);
}

void
CTest::Test1(PRO_INT64 tick)
{
    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        const RPC_ARGUMENT arg(tick);

        IRpcPacket* const request =
            CreateRpcRequest(RPC_FUNCTION_ID1, &arg, 1);
        if (request == NULL)
        {
            return;
        }

        m_client->SendRpcRequest(request);
        request->Release();
    }
}

void
CTest::Test2(PRO_INT32       a,
             PRO_INT32       b,
             const PRO_INT32 c[2],
             PRO_INT64       tick)
{
    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        const RPC_ARGUMENT arg0(a);
        const RPC_ARGUMENT arg1(b);
        const RPC_ARGUMENT arg2(c, 2);
        const RPC_ARGUMENT arg3(tick);

        RPC_ARGUMENT args[4];
        args[0] = arg0;
        args[1] = arg1;
        args[2] = arg2;
        args[3] = arg3;

        IRpcPacket* const request = CreateRpcRequest(
            RPC_FUNCTION_ID2, args, sizeof(args) / sizeof(RPC_ARGUMENT));
        if (request == NULL)
        {
            return;
        }

        m_client->SendRpcRequest(request);
        request->Release();
    }
}

void
CTest::OnLogoff(IRpcClient* client,
                long        errorCode,
                long        sslCode,
                bool        tcpConnected)
{
    assert(client != NULL);
    if (client == NULL)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        if (client != m_client)
        {
            return;
        }

        m_client->Reconnect();
    }
}

void
CTest::OnRpcResult(IRpcClient* client,
                   IRpcPacket* result)
{
    assert(client != NULL);
    assert(result != NULL);
    if (client == NULL || result == NULL)
    {
        return;
    }

    const PRO_UINT32     functionId = result->GetFunctionId();
    const RPC_ERROR_CODE rpcCode    = result->GetRpcCode();

    switch (functionId)
    {
    case RPC_FUNCTION_ID1:
        {
            if (rpcCode == RPCE_OK)
            {
                Test1_ret(client, result);
            }
            else
            {
                Test1_err(client, result);
            }
            break;
        }
    case RPC_FUNCTION_ID2:
        {
            if (rpcCode == RPCE_OK)
            {
                Test2_ret(client, result);
            }
            else
            {
                Test2_err(client, result);
            }
            break;
        }
    }
}

void
CTest::Test1_ret(IRpcClient* client,
                 IRpcPacket* result)
{
    assert(client != NULL);
    assert(result != NULL);

    float tps = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        if (client != m_client)
        {
            return;
        }

        m_stat.PushDataBits(1);
        tps = (float)m_stat.CalcBitRate();
    }

    RPC_ARGUMENT retnArgs[2];
    result->GetArguments(retnArgs, sizeof(retnArgs) / sizeof(RPC_ARGUMENT));

    const bool      arg_ret  = retnArgs[0].bool8Value;
    const PRO_INT64 arg_tick = retnArgs[1].int64Value;

    static PRO_INT64 s_tick = ProGetTickCount64();

    const PRO_INT64 tick = ProGetTickCount64();
    if (tick - s_tick >= 1000)
    {{{
        s_tick = tick;

        printf(
            "\n"
            " CTest::Test1_ret(...), ret : %d, requestId : " PRO_PRT64U ","
            " TPS : %.1f, RTT' : %d \n"
            ,
            (int)arg_ret,
            result->GetRequestId(),
            tps,
            (int)(tick - arg_tick)
            );
    }}}
}

void
CTest::Test2_ret(IRpcClient* client,
                 IRpcPacket* result)
{
    assert(client != NULL);
    assert(result != NULL);

    float tps = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_client == NULL)
        {
            return;
        }

        if (client != m_client)
        {
            return;
        }

        m_stat.PushDataBits(1);
        tps = (float)m_stat.CalcBitRate();
    }

    RPC_ARGUMENT retnArgs[2];
    result->GetArguments(retnArgs, sizeof(retnArgs) / sizeof(RPC_ARGUMENT));

    const PRO_INT32 arg_ret  = retnArgs[0].int32Value;
    const PRO_INT64 arg_tick = retnArgs[1].int64Value;

    static PRO_INT64 s_tick = ProGetTickCount64();

    const PRO_INT64 tick = ProGetTickCount64();
    if (tick - s_tick >= 1000)
    {{{
        s_tick = tick;

        printf(
            "\n"
            " CTest::Test2_ret(...), ret : %d, requestId : " PRO_PRT64U ","
            " TPS : %.1f, RTT' : %d \n"
            ,
            (int)arg_ret,
            result->GetRequestId(),
            tps,
            (int)(tick - arg_tick)
            );
    }}}
}
