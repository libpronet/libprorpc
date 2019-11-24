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
#include "pronet/pro_net.h"
#include "pronet/pro_time_util.h"
#include "pronet/pro_z.h"
#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
////

#define THREAD_COUNT 2

/////////////////////////////////////////////////////////////////////////////
////

int main(int argc, char* argv[])
{
    IProReactor*   reactor      = NULL;
    CTest*         tester       = NULL;
    char           serverIp[64] = "";
    unsigned short serverPort   = 0;

    reactor = ProCreateReactor(THREAD_COUNT);
    if (reactor == NULL)
    {
        printf("\n test_rpc_client --- error! can't create reactor. \n");

        goto EXIT;
    }

    tester = CTest::CreateInstance();
    if (tester == NULL)
    {
        printf("\n test_rpc_client --- error! can't create tester. \n");

        goto EXIT;
    }
    if (!tester->Init(reactor))
    {
        printf("\n test_rpc_client --- error! can't init tester. [cfg?] \n");

        goto EXIT;
    }

    serverPort = tester->GetServerAddr(serverIp);

    printf(
        "\n test_rpc_client --- [server : %s:%u, mmType : %u] --- ok! \n"
        ,
        serverIp,
        (unsigned int)serverPort,
        (unsigned int)tester->GetMmType()
        );

    while (1)
    {
        ProSleep(1);
        tester->Test();
    }

EXIT:

    if (tester != NULL)
    {
        tester->Fini();
        tester->Release();
    }

    ProDeleteReactor(reactor);
    ProSleep(3000);

    return (0);
}
