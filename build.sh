#!/bin/bash

BUILD_DIR=./build/debug

if [ ! -d $BUILD_DIR ]; then
    echo "Creating $BUILD_DIR"
    mkdir $BUILD_DIR
fi

gcc \
src/main.c \
-I "/home/pate/Downloads/vulkan/1.1.126.0/x86_64/include/" \
-DBUILD_DEBUG \
-O0 -fstrict-aliasing -fexceptions \
-g -Wall -Wextra -Wstrict-aliasing \
-Wno-unused-function  -Wno-missing-braces \
-lm -lglfw -lvulkan \
-o $BUILD_DIR/motor

if [ $? -eq 0 ]; then
    echo "Build success"
else
    echo "Build failed"
fi
