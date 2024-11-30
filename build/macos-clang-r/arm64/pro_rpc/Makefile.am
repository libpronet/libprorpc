prolibdir = ${prefix}/libprorpc/lib
proincdir = ${prefix}/libprorpc/include/prorpc

#############################################################################

prolib_PROGRAMS = libpro_rpc.so

proinc_HEADERS = ../../../../src/pro_rpc/pro_rpc.h

libpro_rpc_so_SOURCES = ../../../../src/pro_rpc/pro_rpc.cpp    \
                        ../../../../src/pro_rpc/rpc_client.cpp \
                        ../../../../src/pro_rpc/rpc_packet.cpp \
                        ../../../../src/pro_rpc/rpc_server.cpp

libpro_rpc_so_CPPFLAGS = -DPRO_RPC_EXPORTS             \
                         -I${prefix}/libpronet/include \
                         -I${prefix}/libpromsg/include

libpro_rpc_so_CFLAGS   = -fPIC
libpro_rpc_so_CXXFLAGS = -fPIC

libpro_rpc_so_LDFLAGS = -shared -Wl,-rpath,.:${prefix}/libpronet/lib
libpro_rpc_so_LDADD   =

LIBS = -L${prefix}/libpronet/lib \
       -L${prefix}/libpromsg/lib \
       -lpro_msg                 \
       -lpro_rtp                 \
       -lpro_net                 \
       -lpro_util                \
       -lpro_shared              \
       -lmbedtls                 \
       -lpthread                 \
       -lc
