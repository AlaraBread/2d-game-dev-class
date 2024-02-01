.DEFAULT_GOAL := all
all:
	make -C ./gfc/src
	make -C ./gfc/src static
	make -C ./gfc/simple_json/src
	make -C ./gfc/simple_json/src static
	make -C ./gfc/simple_logger/src
	make -C ./gfc/simple_logger/src static
	make -C ./src

run:
	make all
	./gf2d

grind:
	make all
	valgrind --tool=callgrind ./gf2d