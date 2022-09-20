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

#include "rpc_packet.h"
#include "pro_rpc.h"
#include "pronet/pro_bsd_wrapper.h"
#include "pronet/pro_buffer.h"
#include "pronet/pro_memory_pool.h"
#include "pronet/pro_ref_count.h"
#include "pronet/pro_stl.h"
#include "pronet/pro_z.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
////

static const char      g_s_signature[8]  = "***PRPC";
static PRO_UINT64      g_s_nextRequestId = 1;
static CProThreadMutex g_s_lock;

/////////////////////////////////////////////////////////////////////////////
////

static
PRO_UINT64
MakeRequestId_i()
{
    g_s_lock.Lock();

    const PRO_UINT64 requestId = g_s_nextRequestId;
    ++g_s_nextRequestId;
    if (g_s_nextRequestId == 0)
    {
        ++g_s_nextRequestId;
    }

    g_s_lock.Unlock();

    return (requestId);
}

static
void
Reverse16_i(PRO_UINT16& var)
{
    PRO_UINT16 ret = 0;

    ret |= (var >> 8) & 0x00FF;
    ret |= (var << 8) & 0xFF00;

    var = ret;
}

static
void
Reverse16s_i(PRO_UINT16* vars,
             size_t      size)
{
    if (vars == NULL || size == 0)
    {
        return;
    }

    for (int i = 0; i < (int)size; ++i)
    {
        PRO_UINT16& var = vars[i];
        PRO_UINT16  ret = 0;

        ret |= (var >> 8) & 0x00FF;
        ret |= (var << 8) & 0xFF00;

        var = ret;
    }
}

static
void
Reverse32_i(PRO_UINT32& var)
{
    PRO_UINT32 ret = 0;

    ret |= (var >> 24) & 0x000000FF;
    ret |= (var >>  8) & 0x0000FF00;
    ret |= (var <<  8) & 0x00FF0000;
    ret |= (var << 24) & 0xFF000000;

    var = ret;
}

static
void
Reverse32s_i(PRO_UINT32* vars,
             size_t      size)
{
    if (vars == NULL || size == 0)
    {
        return;
    }

    for (int i = 0; i < (int)size; ++i)
    {
        PRO_UINT32& var = vars[i];
        PRO_UINT32  ret = 0;

        ret |= (var >> 24) & 0x000000FF;
        ret |= (var >>  8) & 0x0000FF00;
        ret |= (var <<  8) & 0x00FF0000;
        ret |= (var << 24) & 0xFF000000;

        var = ret;
    }
}

static
void
Reverse64_i(PRO_UINT64& var)
{
    PRO_UINT64 ret = 0;

    PRO_UINT32 high = (PRO_UINT32)(var >> 32);
    PRO_UINT32 low  = (PRO_UINT32)var;

    Reverse32_i(high);
    Reverse32_i(low);

    ret =   low;
    ret <<= 32;
    ret |=  high;

    var = ret;
}

static
void
Reverse64s_i(PRO_UINT64* vars,
             size_t      size)
{
    if (vars == NULL || size == 0)
    {
        return;
    }

    for (int i = 0; i < (int)size; ++i)
    {
        PRO_UINT64& var = vars[i];
        PRO_UINT64  ret = 0;

        PRO_UINT32 high = (PRO_UINT32)(var >> 32);
        PRO_UINT32 low  = (PRO_UINT32)var;

        Reverse32_i(high);
        Reverse32_i(low);

        ret =   low;
        ret <<= 32;
        ret |=  high;

        var = ret;
    }
}

static
unsigned long
GetNaluSize_i(const RPC_ARGUMENT& arg)
{
    unsigned long size = sizeof(RPC_ARGUMENT);

    switch (arg.type)
    {
    case RPC_DT_BOOL8:
    case RPC_DT_INT8:
    case RPC_DT_UINT8:
    case RPC_DT_INT16:
    case RPC_DT_UINT16:
    case RPC_DT_INT32:
    case RPC_DT_UINT32:
    case RPC_DT_INT64:
    case RPC_DT_UINT64:
    case RPC_DT_FLOAT32:
    case RPC_DT_FLOAT64:
        {
            break;
        }
    case RPC_DT_BOOL8ARRAY:
    case RPC_DT_INT8ARRAY:
    case RPC_DT_UINT8ARRAY:
        {
            if (arg.countForArray > 0 && arg.uint8Values != NULL)
            {
                size += (1 * arg.countForArray + 3) / 4 * 4;
            }
            break;
        }
    case RPC_DT_INT16ARRAY:
    case RPC_DT_UINT16ARRAY:
        {
            if (arg.countForArray > 0 && arg.uint16Values != NULL)
            {
                size += (2 * arg.countForArray + 3) / 4 * 4;
            }
            break;
        }
    case RPC_DT_INT32ARRAY:
    case RPC_DT_UINT32ARRAY:
    case RPC_DT_FLOAT32ARRAY:
        {
            if (arg.countForArray > 0 && arg.uint32Values != NULL)
            {
                size += 4 * arg.countForArray;
            }
            break;
        }
    case RPC_DT_INT64ARRAY:
    case RPC_DT_UINT64ARRAY:
    case RPC_DT_FLOAT64ARRAY:
        {
            if (arg.countForArray > 0 && arg.uint64Values != NULL)
            {
                size += 8 * arg.countForArray;
            }
            break;
        }
    default:
        {
            assert(0);
            size = 0;
            break;
        }
    } /* end of switch (...) */

    return (size);
}

/////////////////////////////////////////////////////////////////////////////
////

CRpcPacket*
CRpcPacket::CreateInstance()
{
    CRpcPacket* const packet = new CRpcPacket(0, 0, false);

    return (packet);
}

CRpcPacket*
CRpcPacket::CreateInstance(PRO_UINT32 functionId,
                           bool       convertByteOrder) /* = false */
{
    assert(functionId > 0);
    if (functionId == 0)
    {
        return (NULL);
    }

    CRpcPacket* const packet =
        new CRpcPacket(MakeRequestId_i(), functionId, convertByteOrder);

    return (packet);
}

CRpcPacket*
CRpcPacket::CreateInstance(PRO_UINT64 requestId,
                           PRO_UINT32 functionId,
                           bool       convertByteOrder) /* = false */
{
    assert(requestId > 0);
    assert(functionId > 0);
    if (requestId == 0 || functionId == 0)
    {
        return (NULL);
    }

    CRpcPacket* const packet =
        new CRpcPacket(requestId, functionId, convertByteOrder);

    return (packet);
}

/*
 * <RPC_HDR> + [args]
 */
bool
CRpcPacket::ParseRpcPacket(const void*                  buffer,
                           size_t                       size,
                           RPC_HDR&                     hdr,
                           CProStlVector<RPC_ARGUMENT>& args)
{
    memset(&hdr, 0, sizeof(RPC_HDR));
    args.clear();

    assert(buffer != NULL);
    assert(size > 0);
    if (buffer == NULL || size == 0)
    {
        return (false);
    }

    bool ret = false;

    do
    {
        unsigned long needSize = sizeof(RPC_HDR);
        const char*   now      = (char*)buffer;

        if (size < needSize)
        {
            break;
        }

        memcpy(&hdr, now, sizeof(RPC_HDR));
        now += sizeof(RPC_HDR);

        hdr.requestId        = pbsd_ntoh64(hdr.requestId);
        hdr.functionId       = pbsd_ntoh32(hdr.functionId);
        hdr.rpcCode          = pbsd_ntoh32(hdr.rpcCode);
        hdr.timeoutInSeconds = pbsd_ntoh32(hdr.timeoutInSeconds);

        if (strncmp(
            hdr.signature, g_s_signature, sizeof(hdr.signature)) != 0 ||
            hdr.requestId == 0 || hdr.functionId == 0)
        {
            break;
        }

        while (1)
        {
            if (size == needSize)
            {
                ret = true; /* Good! */
                break;
            }

            needSize += sizeof(RPC_ARGUMENT);
            if (size < needSize)
            {
                break;
            }

            RPC_ARGUMENT arg;
            memcpy(&arg, now, sizeof(RPC_ARGUMENT));
            now += sizeof(RPC_ARGUMENT);

            bool ret2 = false;

            switch (arg.type)
            {
            case RPC_DT_BOOL8:
            case RPC_DT_INT8:
            case RPC_DT_UINT8:
            case RPC_DT_INT16:
            case RPC_DT_UINT16:
            case RPC_DT_INT32:
            case RPC_DT_UINT32:
            case RPC_DT_INT64:
            case RPC_DT_UINT64:
            case RPC_DT_FLOAT32:
            case RPC_DT_FLOAT64:
                {
                    args.push_back(arg);
                    ret2 = true;
                    break;
                }
            case RPC_DT_BOOL8ARRAY:
            case RPC_DT_INT8ARRAY:
            case RPC_DT_UINT8ARRAY:
                {
                    arg.countForArray = pbsd_ntoh32(arg.countForArray);
                    if (arg.countForArray == 0)
                    {
                        arg.uint8Values = NULL;
                    }
                    else
                    {
                        const unsigned long bodySize =
                            (1 * arg.countForArray + 3) / 4 * 4;
                        needSize += bodySize;
                        if (size < needSize)
                        {
                            break;
                        }

                        arg.uint8Values = (unsigned char*)now;
                        now += bodySize;
                    }

                    args.push_back(arg);
                    ret2 = true;
                    break;
                }
            case RPC_DT_INT16ARRAY:
            case RPC_DT_UINT16ARRAY:
                {
                    arg.countForArray = pbsd_ntoh32(arg.countForArray);
                    if (arg.countForArray == 0)
                    {
                        arg.uint16Values = NULL;
                    }
                    else
                    {
                        const unsigned long bodySize =
                            (2 * arg.countForArray + 3) / 4 * 4;
                        needSize += bodySize;
                        if (size < needSize)
                        {
                            break;
                        }

                        arg.uint16Values = (PRO_UINT16*)now;
                        now += bodySize;
                    }

                    args.push_back(arg);
                    ret2 = true;
                    break;
                }
            case RPC_DT_INT32ARRAY:
            case RPC_DT_UINT32ARRAY:
            case RPC_DT_FLOAT32ARRAY:
                {
                    arg.countForArray = pbsd_ntoh32(arg.countForArray);
                    if (arg.countForArray == 0)
                    {
                        arg.uint32Values = NULL;
                    }
                    else
                    {
                        const unsigned long bodySize = 4 * arg.countForArray;
                        needSize += bodySize;
                        if (size < needSize)
                        {
                            break;
                        }

                        arg.uint32Values = (PRO_UINT32*)now;
                        now += bodySize;
                    }

                    args.push_back(arg);
                    ret2 = true;
                    break;
                }
            case RPC_DT_INT64ARRAY:
            case RPC_DT_UINT64ARRAY:
            case RPC_DT_FLOAT64ARRAY:
                {
                    arg.countForArray = pbsd_ntoh32(arg.countForArray);
                    if (arg.countForArray == 0)
                    {
                        arg.uint64Values = NULL;
                    }
                    else
                    {
                        const unsigned long bodySize = 8 * arg.countForArray;
                        needSize += bodySize;
                        if (size < needSize)
                        {
                            break;
                        }

                        arg.uint64Values = (PRO_UINT64*)now;
                        now += bodySize;
                    }

                    args.push_back(arg);
                    ret2 = true;
                    break;
                }
            } /* end of switch (...) */

            if (!ret2)
            {
                break;
            }
        } /* end of while (...) */
    }
    while (0);

    if (!ret)
    {
        memset(&hdr, 0, sizeof(RPC_HDR));
        args.clear();
    }

    return (ret);
}

CRpcPacket::CRpcPacket(PRO_UINT64 requestId,
                       PRO_UINT32 functionId,
                       bool       convertByteOrder) /* = false */
                       :
m_convertByteOrder(convertByteOrder)
{
    m_clientId             = 0;
    m_magic1               = 0;
    m_magic2               = 0;

    memset(&m_hdr, 0, sizeof(RPC_HDR));
    m_hdr.requestId        = requestId;
    m_hdr.functionId       = functionId;
    m_hdr.rpcCode          = RPCE_OK;
    m_hdr.noreply          = false;
    m_hdr.timeoutInSeconds = (PRO_UINT32)-1;
}

unsigned long
CRpcPacket::AddRef()
{
    const unsigned long refCount = CProRefCount::AddRef();

    return (refCount);
}

unsigned long
CRpcPacket::Release()
{
    const unsigned long refCount = CProRefCount::Release();

    return (refCount);
}

void
CRpcPacket::SetClientId(PRO_UINT64 clientId)
{
    m_clientId = clientId;
}

PRO_UINT64
CRpcPacket::GetClientId() const
{
    return (m_clientId);
}

void
CRpcPacket::SetRequestId(PRO_UINT64 requestId)
{
    m_hdr.requestId = requestId;

    if (m_buffer.Size() >= sizeof(RPC_HDR))
    {
        RPC_HDR* const hdr = (RPC_HDR*)m_buffer.Data();
        hdr->requestId = pbsd_hton64(requestId);
    }
}

PRO_UINT64
CRpcPacket::GetRequestId() const
{
    return (m_hdr.requestId);
}

void
CRpcPacket::SetFunctionId(PRO_UINT32 functionId)
{
    m_hdr.functionId = functionId;

    if (m_buffer.Size() >= sizeof(RPC_HDR))
    {
        RPC_HDR* const hdr = (RPC_HDR*)m_buffer.Data();
        hdr->functionId = pbsd_hton32(functionId);
    }
}

PRO_UINT32
CRpcPacket::GetFunctionId() const
{
    return (m_hdr.functionId);
}

void
CRpcPacket::SetRpcCode(RPC_ERROR_CODE rpcCode)
{
    m_hdr.rpcCode = rpcCode;

    if (m_buffer.Size() >= sizeof(RPC_HDR))
    {
        RPC_HDR* const hdr = (RPC_HDR*)m_buffer.Data();
        hdr->rpcCode = pbsd_hton32(rpcCode);
    }
}

RPC_ERROR_CODE
CRpcPacket::GetRpcCode() const
{
    return (m_hdr.rpcCode);
}

void
CRpcPacket::SetNoreply(bool noreply)
{
    m_hdr.noreply = noreply;

    if (m_buffer.Size() >= sizeof(RPC_HDR))
    {
        RPC_HDR* const hdr = (RPC_HDR*)m_buffer.Data();
        hdr->noreply = noreply;
    }
}

bool
CRpcPacket::GetNoreply() const
{
    return (m_hdr.noreply);
}

void
CRpcPacket::SetTimeout(PRO_UINT32 timeoutInSeconds)
{
    m_hdr.timeoutInSeconds = timeoutInSeconds;

    if (m_buffer.Size() >= sizeof(RPC_HDR))
    {
        RPC_HDR* const hdr = (RPC_HDR*)m_buffer.Data();
        hdr->timeoutInSeconds = pbsd_hton32(timeoutInSeconds);
    }
}

PRO_UINT32
CRpcPacket::GetTimeout() const
{
    return (m_hdr.timeoutInSeconds);
}

unsigned long
CRpcPacket::GetArgumentCount() const
{
    return ((unsigned long)m_args.size());
}

void
CRpcPacket::GetArgument(unsigned long index,
                        RPC_ARGUMENT* arg) const
{
    assert(arg != NULL);
    if (arg == NULL)
    {
        return;
    }

    arg->Reset();

    assert(index < m_args.size());
    if (index >= m_args.size())
    {
        return;
    }

    *arg = m_args[index];
}

void
CRpcPacket::GetArguments(RPC_ARGUMENT* args,
                         size_t        count) const
{
    assert(args != NULL);
    assert(count > 0);
    if (args == NULL || count == 0)
    {
        return;
    }

    for (int i = 0; i < (int)count; ++i)
    {
        args[i].Reset();
    }

    assert(count >= m_args.size());
    if (count < m_args.size())
    {
        return;
    }

    memcpy(args, &m_args[0], sizeof(RPC_ARGUMENT) * m_args.size());
}

void*
CRpcPacket::GetTotalBuffer()
{
    return (m_buffer.Data());
}

const void*
CRpcPacket::GetTotalBuffer() const
{
    return (m_buffer.Data());
}

unsigned long
CRpcPacket::GetTotalSize() const
{
    return (m_buffer.Size());
}

void
CRpcPacket::SetMagic1(PRO_INT64 magic1)
{
    m_magic1 = magic1;
}

PRO_INT64
CRpcPacket::GetMagic1() const
{
    return (m_magic1);
}

void
CRpcPacket::SetMagic2(PRO_INT64 magic2)
{
    m_magic2 = magic2;
}

PRO_INT64
CRpcPacket::GetMagic2() const
{
    return (m_magic2);
}

void
CRpcPacket::CleanAndBeginPushArgument()
{
    m_args.clear();
    m_buffer.Free();
}

bool
CRpcPacket::PushArgument(RPC_ARGUMENT arg)
{
    return (PushArgument(arg, m_args));
}

bool
CRpcPacket::PushArgument(RPC_ARGUMENT                 arg,
                         CProStlVector<RPC_ARGUMENT>& args)
{
    bool ret = true;

    switch (arg.type)
    {
    case RPC_DT_BOOL8:
    case RPC_DT_INT8:
    case RPC_DT_UINT8:
    case RPC_DT_INT16:
    case RPC_DT_UINT16:
    case RPC_DT_INT32:
    case RPC_DT_UINT32:
    case RPC_DT_INT64:
    case RPC_DT_UINT64:
    case RPC_DT_FLOAT32:
    case RPC_DT_FLOAT64:
        {
            break;
        }
    case RPC_DT_BOOL8ARRAY:
    case RPC_DT_INT8ARRAY:
    case RPC_DT_UINT8ARRAY:
    case RPC_DT_INT16ARRAY:
    case RPC_DT_UINT16ARRAY:
    case RPC_DT_INT32ARRAY:
    case RPC_DT_UINT32ARRAY:
    case RPC_DT_INT64ARRAY:
    case RPC_DT_UINT64ARRAY:
    case RPC_DT_FLOAT32ARRAY:
    case RPC_DT_FLOAT64ARRAY:
        {
            if (arg.countForArray > 0 && arg.uint64Values == NULL)
            {
                ret = false;
            }
            break;
        }
    default:
        {
            assert(0);
            ret = false;
            break;
        }
    } /* end of switch (...) */

    if (ret)
    {
        arg.bigEndian_r = arg.bigEndian_r ? true : false;
        args.push_back(arg);
    }

    return (ret);
}

bool
CRpcPacket::PushArguments(const RPC_ARGUMENT* args,
                          size_t              count)
{
    assert(args != NULL);
    assert(count > 0);
    if (args == NULL || count == 0)
    {
        return (false);
    }

    CProStlVector<RPC_ARGUMENT> args2;

    for (int i = 0; i < (int)count; ++i)
    {
        if (!PushArgument(args[i], args2))
        {
            break;
        }
    }

    if (args2.size() != count)
    {
        return (false);
    }

    m_args.insert(m_args.end(), args2.begin(), args2.end());

    return (true);
}

bool
CRpcPacket::EndPushArgument()
{
    char*                        now = NULL;
    CProStlVector<unsigned long> naluSizes;

    {
        unsigned long totalSize = sizeof(RPC_HDR);

        int       i = 0;
        const int c = (int)m_args.size();

        for (; i < c; ++i)
        {
            const unsigned long size = GetNaluSize_i(m_args[i]);
            naluSizes.push_back(size);
            totalSize += size;
        }

        if (!m_buffer.Resize(totalSize))
        {
            return (false);
        }

        now = (char*)m_buffer.Data();
    }

    {
        RPC_HDR hdr;
        memset(&hdr, 0, sizeof(sizeof(RPC_HDR)));
        strncpy_pro(hdr.signature, sizeof(hdr.signature), g_s_signature);
        hdr.requestId        = pbsd_hton64(m_hdr.requestId);
        hdr.functionId       = pbsd_hton32(m_hdr.functionId);
        hdr.rpcCode          = pbsd_hton32(m_hdr.rpcCode);
        hdr.noreply          = m_hdr.noreply;
        hdr.timeoutInSeconds = pbsd_hton32(m_hdr.timeoutInSeconds);

        memcpy(now, &hdr, sizeof(RPC_HDR));
        now += sizeof(RPC_HDR);
    }

#if defined(PRO_WORDS_BIGENDIAN)
    const bool bigEndian = true;
#else
    const bool bigEndian = false;
#endif

    int       i = 0;
    const int c = (int)m_args.size();

    for (; i < c; ++i)
    {
        RPC_ARGUMENT& srcArg = m_args[i];
        RPC_ARGUMENT  dstArg = srcArg;

        switch (dstArg.type)
        {
        case RPC_DT_BOOL8:
        case RPC_DT_INT8:
        case RPC_DT_UINT8:
            {
                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                break;
            }
        case RPC_DT_INT16:
        case RPC_DT_UINT16:
            {
                if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                {
                    Reverse16_i(dstArg.uint16Value);
                    dstArg.bigEndian_r = bigEndian;
                }

                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                srcArg = dstArg;
                break;
            }
        case RPC_DT_INT32:
        case RPC_DT_UINT32:
        case RPC_DT_FLOAT32:
            {
                if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                {
                    Reverse32_i(dstArg.uint32Value);
                    dstArg.bigEndian_r = bigEndian;
                }

                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                srcArg = dstArg;
                break;
            }
        case RPC_DT_INT64:
        case RPC_DT_UINT64:
        case RPC_DT_FLOAT64:
            {
                if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                {
                    Reverse64_i(dstArg.uint64Value);
                    dstArg.bigEndian_r = bigEndian;
                }

                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                srcArg = dstArg;
                break;
            }
        case RPC_DT_BOOL8ARRAY:
        case RPC_DT_INT8ARRAY:
        case RPC_DT_UINT8ARRAY:
            {
                dstArg.countForArray = pbsd_hton32(dstArg.countForArray);
                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                dstArg.countForArray = srcArg.countForArray;

                if (srcArg.countForArray > 0)
                {
                    memcpy(now + sizeof(RPC_ARGUMENT), srcArg.uint8Values,
                        1 * srcArg.countForArray);
                    dstArg.uint8Values =
                        (unsigned char*)(now + sizeof(RPC_ARGUMENT));
                }

                srcArg = dstArg;
                break;
            }
        case RPC_DT_INT16ARRAY:
        case RPC_DT_UINT16ARRAY:
            {
                dstArg.countForArray = pbsd_hton32(dstArg.countForArray);
                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                dstArg.countForArray = srcArg.countForArray;

                if (srcArg.countForArray > 0)
                {
                    memcpy(now + sizeof(RPC_ARGUMENT), srcArg.uint16Values,
                        2 * srcArg.countForArray);
                    dstArg.uint16Values =
                        (PRO_UINT16*)(now + sizeof(RPC_ARGUMENT));

                    if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                    {
                        Reverse16s_i(
                            (PRO_UINT16*)dstArg.uint16Values,
                            dstArg.countForArray
                            );
                        dstArg.bigEndian_r = bigEndian;
                    }
                }

                srcArg = dstArg;
                break;
            }
        case RPC_DT_INT32ARRAY:
        case RPC_DT_UINT32ARRAY:
        case RPC_DT_FLOAT32ARRAY:
            {
                dstArg.countForArray = pbsd_hton32(dstArg.countForArray);
                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                dstArg.countForArray = srcArg.countForArray;

                if (srcArg.countForArray > 0)
                {
                    memcpy(now + sizeof(RPC_ARGUMENT), srcArg.uint32Values,
                        4 * srcArg.countForArray);
                    dstArg.uint32Values =
                        (PRO_UINT32*)(now + sizeof(RPC_ARGUMENT));

                    if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                    {
                        Reverse32s_i(
                            (PRO_UINT32*)dstArg.uint32Values,
                            dstArg.countForArray
                            );
                        dstArg.bigEndian_r = bigEndian;
                    }
                }

                srcArg = dstArg;
                break;
            }
        case RPC_DT_INT64ARRAY:
        case RPC_DT_UINT64ARRAY:
        case RPC_DT_FLOAT64ARRAY:
            {
                dstArg.countForArray = pbsd_hton32(dstArg.countForArray);
                memcpy(now, &dstArg, sizeof(RPC_ARGUMENT));
                dstArg.countForArray = srcArg.countForArray;

                if (srcArg.countForArray > 0)
                {
                    memcpy(now + sizeof(RPC_ARGUMENT), srcArg.uint64Values,
                        8 * srcArg.countForArray);
                    dstArg.uint64Values =
                        (PRO_UINT64*)(now + sizeof(RPC_ARGUMENT));

                    if (m_convertByteOrder && dstArg.bigEndian_r != bigEndian)
                    {
                        Reverse64s_i(
                            (PRO_UINT64*)dstArg.uint64Values,
                            dstArg.countForArray
                            );
                        dstArg.bigEndian_r = bigEndian;
                    }
                }

                srcArg = dstArg;
                break;
            }
        default:
            {
                assert(0);
                break;
            }
        } /* end of switch (...) */

        now += naluSizes[i];
    } /* end of for (...) */

    return (true);
}

/////////////////////////////////////////////////////////////////////////////
////

bool
CheckRpcDataType(RPC_DATA_TYPE type)
{
    bool ret = false;

    switch (type)
    {
    case RPC_DT_BOOL8:
    case RPC_DT_INT8:
    case RPC_DT_UINT8:
    case RPC_DT_INT16:
    case RPC_DT_UINT16:
    case RPC_DT_INT32:
    case RPC_DT_UINT32:
    case RPC_DT_INT64:
    case RPC_DT_UINT64:
    case RPC_DT_FLOAT32:
    case RPC_DT_FLOAT64:
    case RPC_DT_BOOL8ARRAY:
    case RPC_DT_INT8ARRAY:
    case RPC_DT_UINT8ARRAY:
    case RPC_DT_INT16ARRAY:
    case RPC_DT_UINT16ARRAY:
    case RPC_DT_INT32ARRAY:
    case RPC_DT_UINT32ARRAY:
    case RPC_DT_INT64ARRAY:
    case RPC_DT_UINT64ARRAY:
    case RPC_DT_FLOAT32ARRAY:
    case RPC_DT_FLOAT64ARRAY:
        {
            ret = true;
            break;
        }
    }

    return (ret);
}

bool
CmpRpcArgsTypes(const CProStlVector<RPC_ARGUMENT>&  args,
                const CProStlVector<RPC_DATA_TYPE>& types)
{
    const int c1 = (int)args.size();
    const int c2 = (int)types.size();

    if (c1 != c2)
    {
        return (false);
    }

    bool ret = true;

    for (int i = 0; i < c1; ++i)
    {
        if (args[i].type != types[i])
        {
            ret = false;
            break;
        }
    }

    return (ret);
}

bool
CmpRpcPacketTypes(const IRpcPacket*                   packet,
                  const CProStlVector<RPC_DATA_TYPE>& types)
{
    assert(packet != NULL);
    if (packet == NULL)
    {
        return (false);
    }

    const int c1 = (int)packet->GetArgumentCount();
    const int c2 = (int)types.size();

    if (c1 != c2)
    {
        return (false);
    }

    bool ret = true;

    for (int i = 0; i < c1; ++i)
    {
        RPC_ARGUMENT arg;
        packet->GetArgument(i, &arg);

        if (arg.type != types[i])
        {
            ret = false;
            break;
        }
    }

    return (ret);
}
