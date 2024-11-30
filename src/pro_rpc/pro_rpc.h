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
#elif defined(__MINGW32__) || defined(__CYGWIN__)
#define PRO_RPC_API __declspec(dllexport)
#else
#define PRO_RPC_API __attribute__((visibility("default")))
#endif
#else
#define PRO_RPC_API
#endif

/*
 * [[[[ rpc codes
 */
typedef int32_t RPC_ERROR_CODE;

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

    explicit RPC_ARGUMENT(int16_t var)
    {
        Reset();
        type       = RPC_DT_INT16;
        int16Value = var;
    }

    explicit RPC_ARGUMENT(uint16_t var)
    {
        Reset();
        type        = RPC_DT_UINT16;
        uint16Value = var;
    }

    explicit RPC_ARGUMENT(int32_t var)
    {
        Reset();
        type       = RPC_DT_INT32;
        int32Value = var;
    }

    explicit RPC_ARGUMENT(uint32_t var)
    {
        Reset();
        type        = RPC_DT_UINT32;
        uint32Value = var;
    }

    explicit RPC_ARGUMENT(int64_t var)
    {
        Reset();
        type       = RPC_DT_INT64;
        int64Value = var;
    }

    explicit RPC_ARGUMENT(uint64_t var)
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
            countForArray = (uint32_t)count;
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
            countForArray = (uint32_t)count;
            int8Values    = vars;
        }
        else
        {
            int8Values    = "";
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
            countForArray = (uint32_t)count;
            uint8Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const int16_t* vars,
        size_t         count
        )
    {
        Reset();
        type = RPC_DT_INT16ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
            int16Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const uint16_t* vars,
        size_t          count
        )
    {
        Reset();
        type = RPC_DT_UINT16ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
            uint16Values  = vars;
        }
    }

    RPC_ARGUMENT(
        const int32_t* vars,
        size_t         count
        )
    {
        Reset();
        type = RPC_DT_INT32ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
            int32Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const uint32_t* vars,
        size_t          count
        )
    {
        Reset();
        type = RPC_DT_UINT32ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
            uint32Values  = vars;
        }
    }

    RPC_ARGUMENT(
        const int64_t* vars,
        size_t         count
        )
    {
        Reset();
        type = RPC_DT_INT64ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
            int64Values   = vars;
        }
    }

    RPC_ARGUMENT(
        const uint64_t* vars,
        size_t          count
        )
    {
        Reset();
        type = RPC_DT_UINT64ARRAY;
        if (vars != NULL && count > 0)
        {
            countForArray = (uint32_t)count;
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
            countForArray = (uint32_t)count;
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
            countForArray = (uint32_t)count;
            float64Values = vars;
        }
    }

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
    uint32_t                 countForArray;
    union
    {
        bool                 bool8Value;
        char                 int8Value;
        unsigned char        uint8Value;
        int16_t              int16Value;
        uint16_t             uint16Value;
        int32_t              int32Value;
        uint32_t             uint32Value;
        int64_t              int64Value;
        uint64_t             uint64Value;
        float                float32Value;
        double               float64Value;
        const bool*          bool8Values;
        const char*          int8Values;
        const unsigned char* uint8Values;
        const int16_t*       int16Values;
        const uint16_t*      uint16Values;
        const int32_t*       int32Values;
        const uint32_t*      uint32Values;
        const int64_t*       int64Values;
        const uint64_t*      uint64Values;
        const float*         float32Values;
        const double*        float64Values;
    };
};

struct RPC_HDR
{
    char           signature[8]; /* "***PRPC\0" */
    uint64_t       requestId;    /* > 0 */
    uint32_t       functionId;   /* > 0 */
    RPC_ERROR_CODE rpcCode;
    bool           noreply;
    char           reserved[3];
    uint32_t       timeoutInSeconds;
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcPacket
{
public:

    virtual ~IRpcPacket() {}

    virtual unsigned long AddRef() = 0;

    virtual unsigned long Release() = 0;

    virtual uint64_t GetClientId() const = 0;

    virtual uint64_t GetRequestId() const = 0;

    virtual uint32_t GetFunctionId() const = 0;

    virtual RPC_ERROR_CODE GetRpcCode() const = 0;

    virtual bool GetNoreply() const = 0;

    virtual size_t GetArgumentCount() const = 0;

    virtual void GetArgument(
        unsigned int  index,
        RPC_ARGUMENT* arg
        ) const = 0;

    virtual void GetArguments(
        RPC_ARGUMENT* args,
        size_t        count
        ) const = 0;

    virtual void* GetTotalBuffer() = 0;

    virtual const void* GetTotalBuffer() const = 0;

    virtual size_t GetTotalSize() const = 0;

    virtual void SetMagic1(int64_t magic1) = 0;

    virtual int64_t GetMagic1() const = 0;

    virtual void SetMagic2(int64_t magic2) = 0;

    virtual int64_t GetMagic2() const = 0;

    virtual void SetMagicStr(const char* magicStr) = 0;

    virtual const char* GetMagicStr() const = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcClient
{
public:

    virtual ~IRpcClient() {}

    virtual unsigned long AddRef() = 0;

    virtual unsigned long Release() = 0;

    virtual RTP_MM_TYPE GetMmType() const = 0;

    virtual uint64_t GetClientId() const = 0;

    virtual const char* GetServerIp(char serverIp[64]) const = 0;

    virtual unsigned short GetServerPort() const = 0;

    virtual const char* GetLocalIp(char localIp[64]) const = 0;

    virtual unsigned short GetLocalPort() const = 0;

    virtual RPC_ERROR_CODE RegisterFunction(
        uint32_t             functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        ) = 0;

    virtual void UnregisterFunction(uint32_t functionId) = 0;

    virtual RPC_ERROR_CODE SendRpcRequest(
        IRpcPacket*  request,
        bool         noreply             = false,
        unsigned int rpcTimeoutInSeconds = 0
        ) = 0;

    virtual bool SendMsgToServer(
        const void* buf,
        size_t      size,
        uint16_t    charset
        ) = 0;

    virtual bool SendMsgToClients(
        const void*     buf,
        size_t          size,
        uint16_t        charset,
        const uint64_t* dstClients,
        unsigned char   dstClientCount
        ) = 0;

    virtual bool Reconnect() = 0;

    virtual void SetMagic(int64_t magic) = 0;

    virtual int64_t GetMagic() const = 0;

    virtual void SetMagic2(int64_t magic2) = 0;

    virtual int64_t GetMagic2() const = 0;
};

class IRpcClientObserver
{
public:

    virtual ~IRpcClientObserver() {}

    virtual unsigned long AddRef() = 0;

    virtual unsigned long Release() = 0;

    virtual void OnLogon(
        IRpcClient* client,
        uint64_t    myClientId,
        const char* myPublicIp
        ) = 0;

    virtual void OnLogoff(
        IRpcClient* client,
        int         errorCode,
        int         sslCode,
        bool        tcpConnected
        ) = 0;

    virtual void OnRpcResult(
        IRpcClient* client,
        IRpcPacket* result
        ) = 0;

    virtual void OnRecvMsgFromServer(
        IRpcClient* client,
        const void* buf,
        size_t      size,
        uint16_t    charset
        ) = 0;

    virtual void OnRecvMsgFromClient(
        IRpcClient* client,
        const void* buf,
        size_t      size,
        uint16_t    charset,
        uint64_t    srcClientId
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

class IRpcServer
{
public:

    virtual ~IRpcServer() {}

    virtual unsigned long AddRef() = 0;

    virtual unsigned long Release() = 0;

    virtual RTP_MM_TYPE GetMmType() const = 0;

    virtual unsigned short GetServicePort() const = 0;

    virtual RPC_ERROR_CODE RegisterFunction(
        uint32_t             functionId,
        const RPC_DATA_TYPE* callArgTypes, /* = NULL */
        size_t               callArgCount, /* = 0 */
        const RPC_DATA_TYPE* retnArgTypes, /* = NULL */
        size_t               retnArgCount  /* = 0 */
        ) = 0;

    virtual void UnregisterFunction(uint32_t functionId) = 0;

    virtual RPC_ERROR_CODE SendRpcResult(IRpcPacket* result) = 0;

    virtual bool SendMsgToClients(
        const void*     buf,
        size_t          size,
        uint16_t        charset,
        const uint64_t* dstClients,
        unsigned char   dstClientCount
        ) = 0;

    virtual void KickoutClient(uint64_t clientId) = 0;
};

class IRpcServerObserver
{
public:

    virtual ~IRpcServerObserver() {}

    virtual unsigned long AddRef() = 0;

    virtual unsigned long Release() = 0;

    virtual void OnLogon(
        IRpcServer* server,
        uint64_t    clientId,
        const char* clientPublicIp
        ) = 0;

    virtual void OnLogoff(
        IRpcServer* server,
        uint64_t    clientId,
        int         errorCode,
        int         sslCode
        ) = 0;

    virtual void OnRpcRequest(
        IRpcServer* server,
        IRpcPacket* request
        ) = 0;

    virtual void OnRecvMsg(
        IRpcServer* server,
        const void* buf,
        size_t      size,
        uint16_t    charset,
        uint64_t    srcClientId
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

PRO_RPC_API
IRpcClient*
CreateRpcClient(IRpcClientObserver* observer,
                IProReactor*        reactor,
                const char*         argv0,      /* = NULL */
                const char*         configFileName,
                RTP_MM_TYPE         mmType,     /* = 0 */
                const char*         serverIp,   /* = NULL */
                unsigned short      serverPort, /* = 0 */
                const RTP_MSG_USER* user,       /* = NULL */
                const char*         password,   /* = NULL */
                const char*         localIp);   /* = NULL */

PRO_RPC_API
IRpcClient*
CreateRpcClient2(IRpcClientObserver* observer,
                 IProReactor*        reactor,
                 const char*         argv0,      /* = NULL */
                 const char*         configFileName,
                 RTP_MM_TYPE         mmType,     /* = 0 */
                 const char*         serverIp,   /* = NULL */
                 unsigned short      serverPort, /* = 0 */
                 const RTP_MSG_USER* user,       /* = NULL */
                 const char*         password,   /* = NULL */
                 const char*         localIp,    /* = NULL */
                 int64_t             magic,      /* = 0 */
                 int64_t             magic2);    /* = 0 */

PRO_RPC_API
void
DeleteRpcClient(IRpcClient* client);

PRO_RPC_API
IRpcServer*
CreateRpcServer(IRpcServerObserver* observer,
                IProReactor*        reactor,
                const char*         argv0,           /* = NULL */
                const char*         configFileName,
                RTP_MM_TYPE         mmType,          /* = 0 */
                unsigned short      serviceHubPort); /* = 0 */

PRO_RPC_API
void
DeleteRpcServer(IRpcServer* server);

PRO_RPC_API
IRpcPacket*
CreateRpcRequest(uint32_t            functionId,
                 const RPC_ARGUMENT* args,   /* = NULL */
                 size_t              count); /* = 0 */

PRO_RPC_API
IRpcPacket*
CreateRpcResult(uint64_t            clientId,
                uint64_t            requestId,
                uint32_t            functionId,
                RPC_ERROR_CODE      rpcCode,
                const RPC_ARGUMENT* args,   /* = NULL */
                size_t              count); /* = 0 */

PRO_RPC_API
IRpcPacket*
ParseRpcStreamToPacket(const void* streamBuffer,
                       size_t      streamSize);

/////////////////////////////////////////////////////////////////////////////
////

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* ____PRO_RPC_H____ */
