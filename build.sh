#!/bin/bash
clang main.c \
-Wall -g \
-o main \
-lchipmunk \
-lpthread -lm \
-lSDL_gfx -lSDLmain -lSDL \
&& ./main