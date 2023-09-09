#!/bin/sh
THIS_MOD=$(readlink -f "$0")
THIS_DIR=$(dirname "${THIS_MOD}")

cp "${THIS_DIR}/../../build/linux-gcc-d/x86_64/pro_rpc/libpro_rpc.so"           "${THIS_DIR}/linux-gcc/x86_64/"
cp "${THIS_DIR}/../../build/linux-gcc-d/x86_64/test_rpc_server/test_rpc_server" "${THIS_DIR}/linux-gcc/x86_64/"
cp "${THIS_DIR}/../../build/linux-gcc-d/x86_64/test_rpc_client/test_rpc_client" "${THIS_DIR}/linux-gcc/x86_64/"
