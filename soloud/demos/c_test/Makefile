CC = g++

INC_PATHS = ../../soloud/include
INC_PARAMS = $(foreach d, $(INC_PATHS), -I$d)

CFLAGS = -c -Wall -g -Os ${INC_PARAMS}
LD = $(CC)
LDFLAGS = $(foreach d, $(shell find "../../soloud/src/c_api" -name "*.o"), -L$d) -lm -lstdc++

TARGET = c_test

OBJECTS = $(patsubst %.c, %.o, $(shell find . -name "*.c"))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) `sdl2-config --libs`

%.o: %.c
	$(CC) $(CFLAGS) `sdl2-config --cflags` $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)