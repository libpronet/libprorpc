#!/bin/sh

rm -rf ./pro_rpc/.deps
rm -rf ./test_rpc_server/.deps
rm -rf ./test_rpc_client/.deps
rm -rf ./cfg/.deps

#
# configure.ac ---> aclocal.m4
#
aclocal --force

#
# configure.ac + aclocal.m4 ---> configure
#
autoconf --force

#
# configure.ac + Makefile.am ---> Makefile.in
#
automake --add-missing --force-missing --foreign

#
# Makefile.in ---> Makefile
#
./configure                               \
CPPFLAGS="-DNDEBUG                        \
          -D_GNU_SOURCE                   \
          -D_LIBC_REENTRANT               \
          -D_REENTRANT"                   \
CFLAGS="  -O2 -Wall -march=pentium4 -m32" \
CXXFLAGS="-O2 -Wall -march=pentium4 -m32" \
LDFLAGS="" $@

rm -f ./configure
