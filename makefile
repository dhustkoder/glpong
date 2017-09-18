TARGET = glpong
SRC = src/*.c
CFLAGS = -std=c99 -pedantic-errors -Wall -Wextra
CFLAGS_RELEASE = -O3 -DNDEBUG
CFLAGS_DEBUG = -O0 -g -DDEBUG
LIBS = -lc
ifeq ($(BUILD_TYPE),Debug)
	CFLAGS += $(CFLAGS_DEBUG)
else
	CFLAGS += $(CFLAGS_RELEASE)
endif


all:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(TARGET)
