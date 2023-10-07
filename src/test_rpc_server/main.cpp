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

/////////////////////////////////////////////////////////////////////////////
////

#define DEFAULT_THREAD_COUNT 2

/////////////////////////////////////////////////////////////////////////////
////

int main(int argc, char* argv[])
{
    printf(
        "\n"
        " usage: \n"
        " test_rpc_server [thread_count(1~100)] \n"
        "\n"
        " for example: \n"
        " test_rpc_server \n"
        " test_rpc_server 2 \n"
        );

    unsigned int thread_count = DEFAULT_THREAD_COUNT;
    IProReactor* reactor      = NULL;
    CTest*       tester       = NULL;

    if (argc >= 2)
    {
        int value = atoi(argv[1]);
        if (value > 0 && value <= 100)
        {
            thread_count = value;
        }
    }

    reactor = ProCreateReactor(thread_count);
    if (reactor == NULL)
    {
        printf("\n test_rpc_server --- error! can't create reactor. \n");

        goto EXIT;
    }

    tester = CTest::CreateInstance();
    if (tester == NULL)
    {
        printf("\n test_rpc_server --- error! can't create tester. \n");

        goto EXIT;
    }
    if (!tester->Init(reactor))
    {
        printf("\n test_rpc_server --- error! can't init tester. [cfg?] \n");

        goto EXIT;
    }

    printf(
        "\n test_rpc_server --- [port : %u, mmType : %u] --- ok! \n"
        ,
        (unsigned int)tester->GetServicePort(),
        (unsigned int)tester->GetMmType()
        );
    ProSleep(-1);

EXIT:

    if (tester != NULL)
    {
        tester->Fini();
        tester->Release();
    }

    ProDeleteReactor(reactor);
    ProSleep(3000);

    return 0;
}
