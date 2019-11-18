prolibdir = ${prefix}/libprorpc/lib
proincdir = ${prefix}/libprorpc/include/prorpc

#############################################################################

prolib_PROGRAMS = libpro_rpc.so

proinc_HEADERS = ../../../../src/pro_rpc/pro_rpc.h

libpro_rpc_so_SOURCES = ../../../../src/pro_rpc/pro_rpc.cpp    \
                        ../../../../src/pro_rpc/rpc_client.cpp \
                        ../../../../src/pro_rpc/rpc_packet.cpp \
                        ../../../../src/pro_rpc/rpc_server.cpp

libpro_rpc_so_CPPFLAGS = -DPRO_RPC_EXPORTS              \
                         -I/usr/local/libpronet/include \
                         -I/usr/local/libpromsg/include

libpro_rpc_so_CFLAGS   = -fno-strict-aliasing -fPIC -fvisibility=hidden
libpro_rpc_so_CXXFLAGS = -fno-strict-aliasing -fPIC -fvisibility=hidden

libpro_rpc_so_LDFLAGS = -shared -Wl,-soname,libpro_rpc.so     \
                        -Wl,-rpath,.:/usr/local/libpronet/lib \
                        -Wl,--no-undefined
libpro_rpc_so_LDADD   =

LIBS = -L/usr/local/libpronet/lib \
       -L/usr/local/libpromsg/lib \
       -lpro_msg                  \
       -lpro_rtp                  \
       -lpro_net                  \
       -lpro_util                 \
       -lpro_shared               \
       -lmbedtls                  \
       -lstdc++                   \
       -lrt                       \
       -lpthread                  \
       -lm                        \
       -lgcc                      \
       -lc
