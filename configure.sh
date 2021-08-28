#! /bin/sh/

cmake -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -S . -B out/build/ -G "MinGW Makefiles"

# to change to realse use:
# -DCMAKE_BUILD_TYPE=Release