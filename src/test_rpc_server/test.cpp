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
#include "pronet/pro_thread_mutex.h"
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
    m_server  = NULL;
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
        assert(m_server == NULL);
        if (m_reactor != NULL || m_server != NULL)
        {
            return (false);
        }

        m_server = CreateRpcServer(this, reactor, "rpc_server.cfg", 0, 0);
        if (m_server == NULL)
        {
            return (false);
        }

        RegisterFunctions(m_server);

        m_reactor = reactor;
    }

    return (true);
}

void
CTest::RegisterFunctions(IRpcServer* server)
{
    assert(server != NULL);

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
        server->RegisterFunction(
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
        server->RegisterFunction(
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
    IRpcServer* server = NULL;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_server == NULL)
        {
            return;
        }

        server = m_server;
        m_server = NULL;
        m_reactor = NULL;
    }

    DeleteRpcServer(server);
}

unsigned long
PRO_CALLTYPE
CTest::AddRef()
{
    const unsigned long refCount = CProRefCount::AddRef();

    return (refCount);
}

unsigned long
PRO_CALLTYPE
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

        if (m_server != NULL)
        {
            mmType = m_server->GetMmType();
        }
    }

    return (mmType);
}

unsigned short
CTest::GetServicePort() const
{
    unsigned short servicePort = 0;

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_server != NULL)
        {
            servicePort = m_server->GetServicePort();
        }
    }

    return (servicePort);
}

void
PRO_CALLTYPE
CTest::OnRpcRequest(IRpcServer* server,
                    IRpcPacket* request)
{
    assert(server != NULL);
    assert(request != NULL);
    if (server == NULL || request == NULL)
    {
        return;
    }

    {
        CProThreadMutexGuard mon(m_lock);

        if (m_reactor == NULL || m_server == NULL)
        {
            return;
        }

        if (server != m_server)
        {
            return;
        }
    }

    switch (request->GetFunctionId())
    {
    case RPC_FUNCTION_ID1:
        {
            Test1_req(server, request);
            break;
        }
    case RPC_FUNCTION_ID2:
        {
            Test2_req(server, request);
            break;
        }
    }
}

void
CTest::Test1_req(IRpcServer* server,
                 IRpcPacket* request)
{
    assert(server != NULL);
    assert(request != NULL);

    RPC_ARGUMENT callArg;
    request->GetArgument(0, &callArg);

    const PRO_INT64 arg_tick = callArg.int64Value;

    const RPC_ARGUMENT retnArg0(true);
    const RPC_ARGUMENT retnArg1(arg_tick);

    RPC_ARGUMENT retnArgs[2];
    retnArgs[0] = retnArg0;
    retnArgs[1] = retnArg1;

    IRpcPacket* const result = CreateRpcResult(
        request->GetClientId(),
        request->GetRequestId(),
        request->GetFunctionId(),
        RPCE_OK,
        retnArgs,
        sizeof(retnArgs) / sizeof(RPC_ARGUMENT)
        );
    if (result == NULL)
    {
        return;
    }

    server->SendRpcResult(result);
    result->Release();
}

void
CTest::Test2_req(IRpcServer* server,
                 IRpcPacket* request)
{
    assert(server != NULL);
    assert(request != NULL);

    RPC_ARGUMENT callArgs[4];
    request->GetArguments(callArgs, sizeof(callArgs) / sizeof(RPC_ARGUMENT));

    const PRO_INT32        arg_a    = callArgs[0].int32Value;
    const PRO_INT32        arg_b    = callArgs[1].int32Value;
    const PRO_INT32* const arg_c    = callArgs[2].int32Values;
    const PRO_INT64        arg_tick = callArgs[3].int64Value;

    PRO_INT32 sum = arg_a + arg_b;
    if (arg_c != NULL)
    {
        for (int i = 0; i < (int)callArgs[2].countForArray; ++i)
        {
            sum += arg_c[i];
        }
    }

    const RPC_ARGUMENT retnArg0(sum);
    const RPC_ARGUMENT retnArg1(arg_tick);

    RPC_ARGUMENT retnArgs[2];
    retnArgs[0] = retnArg0;
    retnArgs[1] = retnArg1;

    IRpcPacket* const result = CreateRpcResult(
        request->GetClientId(),
        request->GetRequestId(),
        request->GetFunctionId(),
        RPCE_OK,
        retnArgs,
        sizeof(retnArgs) / sizeof(RPC_ARGUMENT)
        );
    if (result == NULL)
    {
        return;
    }

    server->SendRpcResult(result);
    result->Release();
}
