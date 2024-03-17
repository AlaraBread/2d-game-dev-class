.DEFAULT_GOAL := all
all:
	@${MAKE} -C ./gfc/src
	@${MAKE} -C ./gfc/src static
	@${MAKE} -C ./gfc/simple_json/src
	@${MAKE} -C ./gfc/simple_json/src static
	@${MAKE} -C ./gfc/simple_logger/src
	@${MAKE} -C ./gfc/simple_logger/src static
	@${MAKE} -C ./soloud/build/gmake
	@${MAKE} -C ./src

run:
	@${MAKE} all
	./moshing_simulator

grind:
	@${MAKE} all
	valgrind --tool=callgrind ./moshing_simulator