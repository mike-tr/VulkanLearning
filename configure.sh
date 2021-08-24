#! /bin/sh/

cmake -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -S . -B out/build/ -G "MinGW Makefiles"