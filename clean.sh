#!/bin/sh

cd adbi
sh clean.sh
cd ../

rm -vrf ./hijack/libs ./hijack/obj  ./arthook/core/libs ./arthook/core/obj

rm -vrf  examples/arthook_demo/obj
