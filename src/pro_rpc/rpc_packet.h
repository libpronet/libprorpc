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

class CRpcPacket : public IRpcPacket, public CProRefCount
{
public:

    static CRpcPacket* CreateInstance();

    /*
     * for making request
     */
    static CRpcPacket* CreateInstance(
        uint32_t functionId,
        bool     convertByteOrder /* = false */
        );

    /*
     * for making result
     */
    static CRpcPacket* CreateInstance(
        uint64_t requestId,
        uint32_t functionId,
        bool     convertByteOrder /* = false */
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

    virtual unsigned long AddRef();

    virtual unsigned long Release();

    void SetClientId(uint64_t clientId);

    virtual uint64_t GetClientId() const;

    void SetRequestId(uint64_t requestId);

    virtual uint64_t GetRequestId() const;

    void SetFunctionId(uint32_t functionId);

    virtual uint32_t GetFunctionId() const;

    void SetRpcCode(RPC_ERROR_CODE rpcCode);

    virtual RPC_ERROR_CODE GetRpcCode() const;

    void SetNoreply(bool noreply);

    virtual bool GetNoreply() const;

    void SetTimeout(uint32_t timeoutInSeconds);

    uint32_t GetTimeout() const;

    virtual size_t GetArgumentCount() const;

    virtual void GetArgument(
        unsigned int  index,
        RPC_ARGUMENT* arg
        ) const;

    virtual void GetArguments(
        RPC_ARGUMENT* args,
        size_t        count
        ) const;

    virtual void* GetTotalBuffer();

    virtual const void* GetTotalBuffer() const;

    virtual size_t GetTotalSize() const;

    virtual void SetMagic1(int64_t magic1);

    virtual int64_t GetMagic1() const;

    virtual void SetMagic2(int64_t magic2);

    virtual int64_t GetMagic2() const;

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
        uint64_t requestId,
        uint32_t functionId,
        bool     processByteOrder /* = false */
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
    uint64_t                    m_clientId;
    int64_t                     m_magic1;
    int64_t                     m_magic2;

    RPC_HDR                     m_hdr;
    CProStlVector<RPC_ARGUMENT> m_args;
    CProBuffer                  m_buffer;

    DECLARE_SGI_POOL(0)
};

/////////////////////////////////////////////////////////////////////////////
////

bool
CheckRpcDataType(RPC_DATA_TYPE type);

bool
CmpRpcArgsTypes(const CProStlVector<RPC_ARGUMENT>&  args,
                const CProStlVector<RPC_DATA_TYPE>& types);

bool
CmpRpcPacketTypes(const IRpcPacket*                   packet,
                  const CProStlVector<RPC_DATA_TYPE>& types);

/////////////////////////////////////////////////////////////////////////////
////

#endif /* RPC_PACKET_H */
