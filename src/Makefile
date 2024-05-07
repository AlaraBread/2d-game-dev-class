##############################################################################
#
# The Linux-GCC Makefile
#
##############################################################################


#
# Compiler stuff -- adjust to your system.
#

# Linux
PROJECT = moshing_simulator
CC      = gcc

LIB_LIST = ../gfc/libs/libgfc.a ../gfc/simple_json/libs/libsj.a ../portmidi/libportmidi-static.a ../gfc/simple_logger/libs/libsl.a ../soloud/lib/libsoloud_static.a
OBJECTS = $(patsubst %.c,%.o,$(shell find . -name "*.c")) ../soloud/src/c_api/soloud_c.o

INC_PATHS = ../include ../gfc/include ../gfc/simple_logger/include ../gfc/simple_json/include ../soloud/include ../portmidi/pm_common ../portmidi/porttime
INC_PARAMS =$(foreach d, $(INC_PATHS), -I$d)

SDL_CFLAGS = `sdl2-config --cflags` $(INC_PARAMS)
SDL_LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lpng -ljpeg -lz -lm -lpthread -ldl -lasound -lstdc++
LFLAGS = -g  -o ../$(PROJECT)
CFLAGS = -g  -fPIC -Wall -std=gnu99 -fgnu89-inline -Wno-unknown-pragmas -Wno-variadic-macros

DOXYGEN = doxygen

#
# Targets
#

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LIB_LIST) $(SDL_LDFLAGS) 

docs:
	$(DOXYGEN) doxygen.cfg

sources:
	echo (patsubst %.c,%.o,$(shell find . -name "*.c")) > makefile.sources

makefile.dep: depend

depend:
	@touch makefile.dep
	@-rm makefile.dep
	@echo Creating dependencies.
	@for i in *.c; do $(CC) $(INC) -MM $(INC_PARAMS) $$i; done > makefile.dep
	@echo Done.

clean:
	rm $(shell find . -name "*.o")

count:
	wc -l *.c $(foreach d, $(INC_PATHS), $d/*.h) makefile

#
# Dependencies.
#

include makefile.dep

#
# The default rule.
#

%.o: %.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)


