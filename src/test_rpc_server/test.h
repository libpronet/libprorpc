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
#include "pronet/pro_thread_mutex.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include "../pro_rpc/pro_rpc.h"

/////////////////////////////////////////////////////////////////////////////
////

class CTest : public IRpcServerObserver, public CProRefCount
{
public:

    static CTest* CreateInstance();

    bool Init(IProReactor* reactor);

    void Fini();

    virtual unsigned long PRO_CALLTYPE AddRef();

    virtual unsigned long PRO_CALLTYPE Release();

    RTP_MM_TYPE GetMmType() const;

    unsigned short GetServicePort() const;

private:

    CTest();

    virtual ~CTest();

    virtual void PRO_CALLTYPE OnRequest(
        IRpcServer* server,
        IRpcPacket* request
        );

    static void RegisterFunctions(IRpcServer* server);

    void Test1_req(
        IRpcServer* server,
        IRpcPacket* request
        );

    void Test2_req(
        IRpcServer* server,
        IRpcPacket* request
        );

private:

    IProReactor*            m_reactor;
    IRpcServer*             m_server;
    mutable CProThreadMutex m_lock;

    DECLARE_SGI_POOL(0);
};

/////////////////////////////////////////////////////////////////////////////
////

#endif /* TEST_H */
