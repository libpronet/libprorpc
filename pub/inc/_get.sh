#!/bin/sh

THIS_MOD=$(readlink -f "$0")
THIS_DIR=$(dirname "${THIS_MOD}")

cp "${THIS_DIR}/../../src/pro_rpc/pro_rpc.h" "${THIS_DIR}/prorpc/"
