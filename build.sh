#!/bin/sh

sh clean.sh

cd adbi
sh build.sh
cd ../

cd arthook/core/jni
ndk-build
cd ../../../


cd examples/arthook_demo/jni
ndk-build
cd ../../../



