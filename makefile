TARGET = glpong
SRC = src/*.c
CFLAGS = -std=c99 -pedantic-errors -Wall -Wextra
CFLAGS_RELEASE = -O3 -DNDEBUG
CFLAGS_DEBUG = -O0 -g -DDEBUG -fsanitize=address
LIBS = -lc -lSDL2 -lGL -lGLEW


ifeq ($(BUILD_TYPE),Release)
	CFLAGS += $(CFLAGS_RELEASE)
else
	CFLAGS += $(CFLAGS_DEBUG)
endif


all:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(TARGET)
