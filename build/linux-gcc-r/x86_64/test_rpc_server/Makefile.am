probindir = ${prefix}/libprorpc/bin
prolibdir = ${prefix}/libprorpc/lib

#############################################################################

probin_PROGRAMS = test_rpc_server

test_rpc_server_SOURCES = ../../../../src/test_rpc_server/main.cpp \
                          ../../../../src/test_rpc_server/test.cpp

test_rpc_server_CPPFLAGS = -I${prefix}/libpronet/include

test_rpc_server_CFLAGS   =
test_rpc_server_CXXFLAGS =

test_rpc_server_LDFLAGS = -Wl,-rpath,.:../lib:${prolibdir}:${prefix}/libpronet/lib \
                          -Wl,--no-undefined
test_rpc_server_LDADD   =

LIBS = ../pro_rpc/libpro_rpc.so  \
       -L${prefix}/libpronet/lib \
       -lpro_net                 \
       -lpro_util                \
       -lpro_shared              \
       -lpthread                 \
       -lc
