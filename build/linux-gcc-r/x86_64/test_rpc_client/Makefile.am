probindir = ${prefix}/libprorpc/bin
prolibdir = ${prefix}/libprorpc/lib

#############################################################################

probin_PROGRAMS = test_rpc_client

test_rpc_client_SOURCES = ../../../../src/test_rpc_client/main.cpp \
                          ../../../../src/test_rpc_client/test.cpp

test_rpc_client_CPPFLAGS = -I${prefix}/libpronet/include

test_rpc_client_CFLAGS   =
test_rpc_client_CXXFLAGS =

test_rpc_client_LDFLAGS = -Wl,-rpath,.:../lib:${prolibdir}:${prefix}/libpronet/lib \
                          -Wl,--no-undefined
test_rpc_client_LDADD   =

LIBS = ../pro_rpc/libpro_rpc.so  \
       -L${prefix}/libpronet/lib \
       -lpro_net                 \
       -lpro_util                \
       -lpro_shared              \
       -lpthread                 \
       -lc
