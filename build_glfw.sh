#!/bin/bash

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib

src="app"

clang                           \
-Iglfw                          \
chipmunk_glfw.c ./glfw/glad.c   \
`pkg-config --cflags glfw3 glu` \
-o chipmunk_glfw                \
`pkg-config --libs glfw3 glu` -lm 
