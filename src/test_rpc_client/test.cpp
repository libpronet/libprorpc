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

/////////////////////////////////////////////////////////////////////////////
////

#define RPC_FUNCTION_ID1 1
#define RPC_FUNCTION_ID2 2

/////////////////////////////////////////////////////////////////////////////
////

CTest*
CTest::CreateInstance()
{
    return new CTest;
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
CTest::Init(IProReactor* reactor,
            const char*  argv0) /* = NULL */
{
    assert(reactor != NULL);
    if (reactor == NULL)
    {
        return false;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        assert(m_reactor == NULL);
        assert(m_client == NULL);
        if (m_reactor != NULL || m_client != NULL)
        {
            return false;
        }

        m_client = CreateRpcClient(
            this,
            reactor,
            argv0,
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
            return false;
        }

        RegisterFunctions(m_client);

        m_reactor = reactor;
    }

    return true;
}

void
CTest::RegisterFunctions(IRpcClient* client)
{
    assert(client != NULL);

    /*
     * function1
     *
     * bool ReturnTrue(int64_t& tick);
     */
    {
        RPC_DATA_TYPE callArgTypes[] =
        {
            RPC_DT_INT64
        };
        RPC_DATA_TYPE retnArgTypes[] =
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
     * int32_t Sum(int32_t a, int32_t b, const int32_t c[2], int64_t& tick);
     */
    {
        RPC_DATA_TYPE callArgTypes[] =
        {
            RPC_DT_INT32,
            RPC_DT_INT32,
            RPC_DT_INT32ARRAY,
            RPC_DT_INT64
        };
        RPC_DATA_TYPE retnArgTypes[] =
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
    return CProRefCount::AddRef();
}

unsigned long
CTest::Release()
{
    return CProRefCount::Release();
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

    return mmType;
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

    return serverPort;
}

void
CTest::Test()
{
    int64_t tick = ProGetTickCount64();

    Test1(tick);

    int32_t a    = 0;
    int32_t b    = 1;
    int32_t c[2] = { 2, 3 };
    Test2(a, b, c, tick);
}

void
CTest::Test1(int64_t tick)
{
    CProThreadMutexGuard mon(m_lock);

    if (m_reactor == NULL || m_client == NULL)
    {
        return;
    }

    RPC_ARGUMENT arg(tick);

    IRpcPacket* request = CreateRpcRequest(RPC_FUNCTION_ID1, &arg, 1);
    if (request == NULL)
    {
        return;
    }

    m_client->SendRpcRequest(request);
    request->Release();
}

void
CTest::Test2(int32_t       a,
             int32_t       b,
             const int32_t c[2],
             int64_t       tick)
{
    CProThreadMutexGuard mon(m_lock);

    if (m_reactor == NULL || m_client == NULL)
    {
        return;
    }

    RPC_ARGUMENT arg0(a);
    RPC_ARGUMENT arg1(b);
    RPC_ARGUMENT arg2(c, 2);
    RPC_ARGUMENT arg3(tick);

    RPC_ARGUMENT args[4];
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;

    IRpcPacket* request = CreateRpcRequest(
        RPC_FUNCTION_ID2, args, sizeof(args) / sizeof(RPC_ARGUMENT));
    if (request == NULL)
    {
        return;
    }

    m_client->SendRpcRequest(request);
    request->Release();
}

void
CTest::OnLogoff(IRpcClient* client,
                int         errorCode,
                int         sslCode,
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

    uint32_t       functionId = result->GetFunctionId();
    RPC_ERROR_CODE rpcCode    = result->GetRpcCode();

    switch (functionId)
    {
    case RPC_FUNCTION_ID1:
        if (rpcCode == RPCE_OK)
        {
            Test1_ret(client, result);
        }
        else
        {
            Test1_err(client, result);
        }
        break;
    case RPC_FUNCTION_ID2:
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

    bool    arg_ret  = retnArgs[0].bool8Value;
    int64_t arg_tick = retnArgs[1].int64Value;

    static int64_t s_tick = ProGetTickCount64();

    int64_t tick = ProGetTickCount64();
    if (tick - s_tick >= 1000)
    {{{
        s_tick = tick;

        printf(
            "\n"
            " CTest::Test1_ret(), ret : %d, requestId : %llu, TPS : %.1f, RTT' : %d \n"
            ,
            (int)arg_ret,
            (unsigned long long)result->GetRequestId(),
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

    int32_t arg_ret  = retnArgs[0].int32Value;
    int64_t arg_tick = retnArgs[1].int64Value;

    static int64_t s_tick = ProGetTickCount64();

    int64_t tick = ProGetTickCount64();
    if (tick - s_tick >= 1000)
    {{{
        s_tick = tick;

        printf(
            "\n"
            " CTest::Test2_ret(), ret : %d, requestId : %llu, TPS : %.1f, RTT' : %d \n"
            ,
            (int)arg_ret,
            (unsigned long long)result->GetRequestId(),
            tps,
            (int)(tick - arg_tick)
            );
    }}}
}
