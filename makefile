TARGET = glpong
SRC = src/*.c
CFLAGS = -std=c99 -pedantic-errors -Wall -Wextra
CFLAGS_RELEASE = -O3 -DNDEBUG
CFLAGS_DEBUG = -O0 -g -DDEBUG -fsanitize=address
LIBS = -lc -lm -lSDL2 -lSDL2_mixer -lGL -lGLEW


ifeq ($(BUILD_TYPE),Release)
	CFLAGS += $(CFLAGS_RELEASE)
else
	CFLAGS += $(CFLAGS_DEBUG)
endif


all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)
