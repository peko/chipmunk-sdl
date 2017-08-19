#!/bin/bash
clang chipmunk_sdl.c space.c \
-I/usr/include/SDL \
-Wall -g \
-o chipmunk_sdl \
-lchipmunk \
-lpthread -lm \
-lSDL_gfx -lSDLmain -lSDL \
&& ./chipmunk_sdl
