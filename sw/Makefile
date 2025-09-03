DEVICE=/dev/ttyACM0

help:
	@echo "make targets:"
	@echo "  flash    - build and flash esphome firmware, then open log monitoring"
	@echo "  fw       - build esphome firmware"
	@echo "  test     - compile the pc build and run integration tests"
	@echo "  interact - compile the pc build and run it in interactive mode"
	@echo "  format   - format project's C++ code using clang-format"

flash:
	src-esphome/esphome.sh run --device ${DEVICE} src-esphome/config.yaml

fw:
	src-esphome/esphome.sh compile src-esphome/config.yaml
	@echo
	@echo "Factory firmware:"
	@ls ${PWD}/src-esphome/.esphome/build/ant/.pioenvs/ant/firmware.factory.bin
	@echo
	@echo "OTA firmware:"
	@ls ${PWD}/src-esphome/.esphome/build/ant/.pioenvs/ant/firmware.ota.bin

test:
	src-pc/build.sh
	src-pc/tests/test.sh

interact:
	src-pc/run.sh

format:
	clang-format -i src-common/*.cpp src-common/*.hpp src-pc/*.cpp src-pc/*.hpp
