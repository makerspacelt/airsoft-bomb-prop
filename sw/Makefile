help:
	@echo "make targets:"
	@echo "  flash    - build and flash esphome firmware, then open log monitoring"
	@echo "  fw       - build esphome firmware"
	@echo "  test     - compile the pc build and run integration tests"
	@echo "  interact - compile the pc build and run it in interactive mode"

flash:
	src-esphome/esphome.sh run src-esphome/config.yaml

fw:
	src-esphome/esphome.sh compile src-esphome/config.yaml

test:
	src-pc/build.sh
	src-pc/tests/test.sh

interact:
	src-pc/run.sh

