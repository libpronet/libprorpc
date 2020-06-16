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

#if !defined(RPC_PACKET_H)
#define RPC_PACKET_H

#include "pro_rpc.h"
#include "pronet/pro_buffer.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_ref_count.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_z.h"

/////////////////////////////////////////////////////////////////////////////
////

struct RPC_HDR
{
    char           signature[8];
    PRO_UINT64     requestId;
    PRO_UINT32     functionId;
    RPC_ERROR_CODE rpcCode; /* PRO_INT32 */
    bool           noreply;
    char           reserved[3];
    PRO_UINT32     timeoutInSeconds;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

class CRpcPacket : public IRpcPacket, public CProRefCount
{
public:

    static CRpcPacket* CreateInstance();

    /*
     * for making request
     */
    static CRpcPacket* CreateInstance(
        PRO_UINT32 functionId,
        bool       convertByteOrder /* = false */
        );

    /*
     * for making result
     */
    static CRpcPacket* CreateInstance(
        PRO_UINT64 requestId,
        PRO_UINT32 functionId,
        bool       convertByteOrder /* = false */
        );

    /*
     * <RPC_HDR> + [args]
     */
    static bool ParseRpcPacket(
        const void*                  buffer,
        size_t                       size,
        RPC_HDR&                     hdr,
        CProStlVector<RPC_ARGUMENT>& args
        );

    virtual unsigned long PRO_CALLTYPE AddRef();

    virtual unsigned long PRO_CALLTYPE Release();

    void SetClientId(PRO_UINT64 clientId);

    virtual PRO_UINT64 PRO_CALLTYPE GetClientId() const;

    void SetRequestId(PRO_UINT64 requestId);

    virtual PRO_UINT64 PRO_CALLTYPE GetRequestId() const;

    void SetFunctionId(PRO_UINT32 functionId);

    virtual PRO_UINT32 PRO_CALLTYPE GetFunctionId() const;

    void SetRpcCode(RPC_ERROR_CODE rpcCode);

    virtual RPC_ERROR_CODE PRO_CALLTYPE GetRpcCode() const;

    void SetNoreply(bool noreply);

    virtual bool PRO_CALLTYPE GetNoreply() const;

    void SetTimeout(PRO_UINT32 timeoutInSeconds);

    PRO_UINT32 GetTimeout() const;

    virtual unsigned long PRO_CALLTYPE GetArgumentCount() const;

    virtual void PRO_CALLTYPE GetArgument(
        unsigned long index,
        RPC_ARGUMENT* arg
        ) const;

    virtual void PRO_CALLTYPE GetArguments(
        RPC_ARGUMENT* args,
        size_t        count
        ) const;

    virtual void* PRO_CALLTYPE GetTotalBuffer();

    virtual const void* PRO_CALLTYPE GetTotalBuffer() const;

    virtual unsigned long PRO_CALLTYPE GetTotalSize() const;

    virtual void PRO_CALLTYPE SetMagic1(PRO_INT64 magic1);

    virtual PRO_INT64 PRO_CALLTYPE GetMagic1() const;

    virtual void PRO_CALLTYPE SetMagic2(PRO_INT64 magic2);

    virtual PRO_INT64 PRO_CALLTYPE GetMagic2() const;

    /*
     * [[[[ push arguments
     */
    void CleanAndBeginPushArgument();

    bool PushArgument(RPC_ARGUMENT arg);

    bool PushArguments(
        const RPC_ARGUMENT* args,
        size_t              count
        );

    bool EndPushArgument();
    /*
     * ]]]]
     */

private:

    CRpcPacket(
        PRO_UINT64 requestId,
        PRO_UINT32 functionId,
        bool       processByteOrder /* = false */
        );

    virtual ~CRpcPacket()
    {
    }

    static bool PushArgument(
        RPC_ARGUMENT                 arg,
        CProStlVector<RPC_ARGUMENT>& args
        );

private:

    const bool                  m_convertByteOrder;
    PRO_UINT64                  m_clientId;
    PRO_INT64                   m_magic1;
    PRO_INT64                   m_magic2;

    RPC_HDR                     m_hdr;
    CProStlVector<RPC_ARGUMENT> m_args;
    CProBuffer                  m_buffer;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

bool
PRO_CALLTYPE
CheckRpcDataType(RPC_DATA_TYPE type);

bool
PRO_CALLTYPE
CmpRpcArgsTypes(const CProStlVector<RPC_ARGUMENT>&  args,
                const CProStlVector<RPC_DATA_TYPE>& types);

bool
PRO_CALLTYPE
CmpRpcPacketTypes(const IRpcPacket*                   packet,
                  const CProStlVector<RPC_DATA_TYPE>& types);

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_PACKET_H */
