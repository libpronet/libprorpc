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

#if !defined(____PRO_RPC_H____)
#define ____PRO_RPC_H____

#include "pronet/rtp_base.h"
#include "pronet/rtp_msg.h"

#if defined(__cplusplus)
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
////

#if defined(PRO_RPC_EXPORTS)
#if defined(_MSC_VER)
#define PRO_RPC_API /* using xxx.def */
#else
#define PRO_RPC_API PRO_EXPORT
#endif
#else
#define PRO_RPC_API PRO_IMPORT
#endif

/*
 * [[[[ rpc codes
 */
typedef PRO_INT32 RPC_ERROR_CODE;

static const RPC_ERROR_CODE RPCE_OK                    =     0;
static const RPC_ERROR_CODE RPCE_ERROR                 =    -1;
static const RPC_ERROR_CODE RPCE_NOT_ENOUGH_MEMORY     =    -2;
static const RPC_ERROR_CODE RPCE_MISMATCHED_PARAMETER  = -1001;
static const RPC_ERROR_CODE RPCE_INVALID_ARGUMENT      = -1002;
static const RPC_ERROR_CODE RPCE_INVALID_FUNCTION      = -1003;
static const RPC_ERROR_CODE RPCE_CLIENT_BUSY           = -1088;
static const RPC_ERROR_CODE RPCE_SERVER_BUSY           = -1099;
static const RPC_ERROR_CODE RPCE_NETWORK_NOT_CONNECTED = -2001;
static const RPC_ERROR_CODE RPCE_NETWORK_BROKEN        = -2054;
static const RPC_ERROR_CODE RPCE_NETWORK_TIMEOUT       = -2060;
static const RPC_ERROR_CODE RPCE_NETWORK_BUSY          = -2099;
/*
 * ]]]]
 */

/*
 * [[[[ data types
 */
typedef unsigned char RPC_DATA_TYPE;

static const RPC_DATA_TYPE RPC_DT_BOOL8        =   1;
static const RPC_DATA_TYPE RPC_DT_INT8         =   7;
static const RPC_DATA_TYPE RPC_DT_UINT8        =   8;
static const RPC_DATA_TYPE RPC_DT_INT16        =  15;
static const RPC_DATA_TYPE RPC_DT_UINT16       =  16;
static const RPC_DATA_TYPE RPC_DT_INT32        =  31;
static const RPC_DATA_TYPE RPC_DT_UINT32       =  32;
static const RPC_DATA_TYPE RPC_DT_INT64        =  63;
static const RPC_DATA_TYPE RPC_DT_UINT64       =  64;
static const RPC_DATA_TYPE RPC_DT_FLOAT32      =  65;
static const RPC_DATA_TYPE RPC_DT_FLOAT64      =  66;
static const RPC_DATA_TYPE RPC_DT_BOOL8ARRAY   = 101;
static const RPC_DATA_TYPE RPC_DT_INT8ARRAY    = 107;
static const RPC_DATA_TYPE RPC_DT_UINT8ARRAY   = 108;
static const RPC_DATA_TYPE RPC_DT_INT16ARRAY   = 115;
static const RPC_DATA_TYPE RPC_DT_UINT16ARRAY  = 116;
static const RPC_DATA_TYPE RPC_DT_INT32ARRAY   = 131;
static const RPC_DATA_TYPE RPC_DT_UINT32ARRAY  = 132;
static const RPC_DATA_TYPE RPC_DT_INT64ARRAY   = 163;
static const RPC_DATA_TYPE RPC_DT_UINT64ARRAY  = 164;
static const RPC_DATA_TYPE RPC_DT_FLOAT32ARRAY = 165;
static const RPC_DATA_TYPE RPC_DT_FLOAT64ARRAY = 166;
/*
 * ]]]]
 */

struct RPC_ARGUMENT
{
    RPC_ARGUMENT()
    {
        Reset();
    }

    explicit RPC_ARGUMENT(bool var)
    {
        Reset();
        type       = RPC_DT_BOOL8;
        bool8Value = var;
    }

    explicit RPC_ARGUMENT(char var)
    {
        Reset();
        type      = RPC_DT_INT8;
        int8Value = var;
    }

    explicit RPC_ARGUMENT(unsigned char var)
    {
        Reset();
        type       = RPC_DT_UINT8;
        uint8Value = var;
    }

    explicit RPC_ARGUMENT(PRO_INT16 var)
    {
        Reset();
        type       = RPC_DT_INT16;
        int16Value = var;
    }

    explicit RPC_ARGUMENT(PRO_UINT16 var)
    {
        Reset();
        type        = RPC_DT_UINT16;
        uint16Value = var;
    }

    explicit RPC_ARGUMENT(PRO_INT32 var)
    {
        Reset();
        type       = RPC_DT_INT32;
        int32Value = var;
    }

    explicit RPC_ARGUMENT(PRO_UINT32 var)
    {
        Reset();
        type        = RPC_DT_UINT32;
        uint32Value = var;
    }

    explicit RPC_ARGUMENT(PRO_INT64 var)
    {
        Reset();
        type       = RPC_DT_INT64;
        int64Value = var;
    }

    explicit RPC_ARGUMENT(PRO_UINT64 var)
    {
        Reset();
        type        = RPC_DT_UINT64;
        uint64Value = var;
    }

    explicit RPC_ARGUMENT(float var)
    {
        Reset();
        type         = RPC_DT_FLOAT32;
        float32Value = var;
    }

    explicit RPC_ARGUMENT(double var)
    {
        Reset();
        type         = RPC_DT_FLOAT64;
        float64Value = var;
    }

    RPC_ARGUMENT(
        const bool* vars,
        size_t      count
        )
    {
        Reset();
        type = RPC_DT_BOOL8ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            bool8Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const char* vars,
        size_t      count
        )
    {
        Reset();
        type = RPC_DT_INT8ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            int8Values    = vars;
        }
    }

    RPC_ARGUMENT(
        const unsigned char* vars,
        size_t               count
        )
    {
        Reset();
        type = RPC_DT_UINT8ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            uint8Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_INT16* vars,
        size_t           count
        )
    {
        Reset();
        type = RPC_DT_INT16ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            int16Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_UINT16* vars,
        size_t            count
        )
    {
        Reset();
        type = RPC_DT_UINT16ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            uint16Values  = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_INT32* vars,
        size_t           count
        )
    {
        Reset();
        type = RPC_DT_INT32ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            int32Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_UINT32* vars,
        size_t            count
        )
    {
        Reset();
        type = RPC_DT_UINT32ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            uint32Values  = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_INT64* vars,
        size_t           count
        )
    {
        Reset();
        type = RPC_DT_INT64ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            int64Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const PRO_UINT64* vars,
        size_t            count
        )
    {
        Reset();
        type = RPC_DT_UINT64ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            uint64Values  = vars;
        }
    }

    RPC_ARGUMENT(
        const float* vars,
        size_t       count
        )
    {
        Reset();
        type = RPC_DT_FLOAT32ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            float32Values = vars;
        }
    }

    RPC_ARGUMENT(
        const double* vars,
        size_t        count
        )
    {
        Reset();
        type = RPC_DT_FLOAT64ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (PRO_UINT32)count;
            float64Values = vars;
        }
    }

    /*
     * desabled!!!
     */
    RPC_ARGUMENT(long);
    RPC_ARGUMENT(unsigned long);
    RPC_ARGUMENT(const long*, size_t);
    RPC_ARGUMENT(const unsigned long*, size_t);
    RPC_ARGUMENT(void*);
    RPC_ARGUMENT(void*, size_t);

    void Reset()
    {
#if defined(PRO_WORDS_BIGENDIAN)
        bigEndian_r   = true;
#else
        bigEndian_r   = false;
#endif
        type          = 0;
        reserved[0]   = 0;
        reserved[1]   = 0;
        countForArray = 0;
        uint64Value   = 0;
    }

    bool                     bigEndian_r;
    RPC_DATA_TYPE            type;
    char                     reserved[2];
    PRO_UINT32               countForArray;
    union
    {
        bool                 bool8Value;
        char                 int8Value;
        unsigned char        uint8Value;
        PRO_INT16            int16Value;
        PRO_UINT16           uint16Value;
        PRO_INT32            int32Value;
        PRO_UINT32           uint32Value;
        PRO_INT64            int64Value;
        PRO_UINT64           uint64Value;
        float                float32Value;
        double               float64Value;
        const bool*          bool8Values;
        const char*          int8Values;
        const unsigned char* uint8Values;
        const PRO_INT16*     int16Values;
        const PRO_UINT16*    uint16Values;
        const PRO_INT32*     int32Values;
        const PRO_UINT32*    uint32Values;
        const PRO_INT64*     int64Values;
        const PRO_UINT64*    uint64Values;
        const float*         float32Values;
        const double*        float64Values;
    };
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcPacket
{
public:

    virtual ~IRpcPacket() {}

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual PRO_UINT64 PRO_CALLTYPE GetClientId() const = 0;

    virtual PRO_UINT64 PRO_CALLTYPE GetRequestId() const = 0;

    virtual PRO_UINT32 PRO_CALLTYPE GetFunctionId() const = 0;

    virtual RPC_ERROR_CODE PRO_CALLTYPE GetRpcCode() const = 0;

    virtual bool PRO_CALLTYPE GetNoreply() const = 0;

    virtual unsigned long PRO_CALLTYPE GetArgumentCount() const = 0;

    virtual void PRO_CALLTYPE GetArgument(
        unsigned long index,
        RPC_ARGUMENT* arg
        ) const = 0;

    virtual void PRO_CALLTYPE GetArguments(
        RPC_ARGUMENT* args,
        size_t        count
        ) const = 0;

    virtual void* PRO_CALLTYPE GetTotalBuffer() = 0;

    virtual const void* PRO_CALLTYPE GetTotalBuffer() const = 0;

    virtual unsigned long PRO_CALLTYPE GetTotalSize() const = 0;

    virtual void PRO_CALLTYPE SetMagic1(PRO_INT64 magic1) = 0;

    virtual PRO_INT64 PRO_CALLTYPE GetMagic1() const = 0;

    virtual void PRO_CALLTYPE SetMagic2(PRO_INT64 magic2) = 0;

    virtual PRO_INT64 PRO_CALLTYPE GetMagic2() const = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcClient
{
public:

    virtual ~IRpcClient() {}

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual RTP_MM_TYPE PRO_CALLTYPE GetMmType() const = 0;

    virtual PRO_UINT64 PRO_CALLTYPE GetClientId() const = 0;

    virtual const char* PRO_CALLTYPE GetServerIp(char serverIp[64]) const = 0;

    virtual unsigned short PRO_CALLTYPE GetServerPort() const = 0;

    virtual const char* PRO_CALLTYPE GetLocalIp(char localIp[64]) const = 0;

    virtual unsigned short PRO_CALLTYPE GetLocalPort() const = 0;

    virtual RPC_ERROR_CODE PRO_CALLTYPE RegisterFunction(
        PRO_UINT32           functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        ) = 0;

    virtual void PRO_CALLTYPE UnregisterFunction(PRO_UINT32 functionId) = 0;

    virtual RPC_ERROR_CODE PRO_CALLTYPE SendRpcRequest(
        IRpcPacket*   request,
        bool          noreply             = false,
        unsigned long rpcTimeoutInSeconds = 0
        ) = 0;

    virtual bool PRO_CALLTYPE SendMsgToServer(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset
        ) = 0;

    virtual bool PRO_CALLTYPE SendMsgToClients(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset,
        const PRO_UINT64* dstClients,
        unsigned char     dstClientCount
        ) = 0;

    virtual bool PRO_CALLTYPE Reconnect() = 0;

    virtual void PRO_CALLTYPE SetMagic(PRO_INT64 magic) = 0;

    virtual PRO_INT64 PRO_CALLTYPE GetMagic() const = 0;
};

class IRpcClientObserver
{
public:

    virtual ~IRpcClientObserver() {}

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual void PRO_CALLTYPE OnLogon(
        IRpcClient* client,
        PRO_UINT64  myClientId,
        const char* myPublicIp
        ) = 0;

    virtual void PRO_CALLTYPE OnLogoff(
        IRpcClient* client,
        long        errorCode,
        long        sslCode,
        bool        tcpConnected
        ) = 0;

    virtual void PRO_CALLTYPE OnRpcResult(
        IRpcClient* client,
        IRpcPacket* result
        ) = 0;

    virtual void PRO_CALLTYPE OnRecvMsgFromServer(
        IRpcClient*   client,
        const void*   buf,
        unsigned long size,
        PRO_UINT16    charset
        ) = 0;

    virtual void PRO_CALLTYPE OnRecvMsgFromClient(
        IRpcClient*   client,
        const void*   buf,
        unsigned long size,
        PRO_UINT16    charset,
        PRO_UINT64    srcClientId
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcServer
{
public:

    virtual ~IRpcServer() {}

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual RTP_MM_TYPE PRO_CALLTYPE GetMmType() const = 0;

    virtual unsigned short PRO_CALLTYPE GetServicePort() const = 0;

    virtual RPC_ERROR_CODE PRO_CALLTYPE RegisterFunction(
        PRO_UINT32           functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        ) = 0;

    virtual void PRO_CALLTYPE UnregisterFunction(PRO_UINT32 functionId) = 0;

    virtual RPC_ERROR_CODE PRO_CALLTYPE SendRpcResult(IRpcPacket* result) = 0;

    virtual bool PRO_CALLTYPE SendMsgToClients(
        const void*       buf,
        unsigned long     size,
        PRO_UINT16        charset,
        const PRO_UINT64* dstClients,
        unsigned char     dstClientCount
        ) = 0;

    virtual void PRO_CALLTYPE KickoutClient(PRO_UINT64 clientId) = 0;
};

class IRpcServerObserver
{
public:

    virtual ~IRpcServerObserver() {}

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual void PRO_CALLTYPE OnLogon(
        IRpcServer* server,
        PRO_UINT64  clientId,
        const char* clientPublicIp
        ) = 0;

    virtual void PRO_CALLTYPE OnLogoff(
        IRpcServer* server,
        PRO_UINT64  clientId,
        long        errorCode,
        long        sslCode
        ) = 0;

    virtual void PRO_CALLTYPE OnRpcRequest(
        IRpcServer* server,
        IRpcPacket* request
        ) = 0;

    virtual void PRO_CALLTYPE OnRecvMsg(
        IRpcServer*   server,
        const void*   buf,
        unsigned long size,
        PRO_UINT16    charset,
        PRO_UINT64    srcClientId
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

PRO_RPC_API
IRpcClient*
PRO_CALLTYPE
CreateRpcClient(IRpcClientObserver* observer,
                IProReactor*        reactor,
                const char*         configFileName,
                RTP_MM_TYPE         mmType,     /* = 0 */
                const char*         serverIp,   /* = NULL */
                unsigned short      serverPort, /* = 0 */
                const RTP_MSG_USER* user,       /* = NULL */
                const char*         password,   /* = NULL */
                const char*         localIp);   /* = NULL */

PRO_RPC_API
void
PRO_CALLTYPE
DeleteRpcClient(IRpcClient* client);

PRO_RPC_API
IRpcServer*
PRO_CALLTYPE
CreateRpcServer(IRpcServerObserver* observer,
                IProReactor*        reactor,
                const char*         configFileName,
                RTP_MM_TYPE         mmType,          /* = 0 */
                unsigned short      serviceHubPort); /* = 0 */

PRO_RPC_API
void
PRO_CALLTYPE
DeleteRpcServer(IRpcServer* server);

PRO_RPC_API
IRpcPacket*
PRO_CALLTYPE
CreateRpcRequest(PRO_UINT32          functionId,
                 const RPC_ARGUMENT* args,   /* = NULL */
                 size_t              count); /* = 0 */

PRO_RPC_API
IRpcPacket*
PRO_CALLTYPE
CreateRpcResult(PRO_UINT64          clientId,
                PRO_UINT64          requestId,
                PRO_UINT32          functionId,
                RPC_ERROR_CODE      rpcCode,
                const RPC_ARGUMENT* args,   /* = NULL */
                size_t              count); /* = 0 */

/////////////////////////////////////////////////////////////////////////////
////

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* ____PRO_RPC_H____ */
