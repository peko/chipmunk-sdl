#!/bin/bash
#!/bin/bash
src="app"
clang \
-Iglfw \
chipmunk_glfw.c ./glfw/glad.c \
`pkg-config --cflags glfw3 glu` \
-o chipmunk_glfw \
`pkg-config --libs glfw3 glu` -lm