DEVICE=/dev/ttyACM0

help:
	@echo "make targets:"
	@echo "  flash    - build and flash esphome firmware, then open log monitoring"
	@echo "  fw       - build esphome firmware"
	@echo "  test     - compile the pc build and run integration tests"
	@echo "  interact - compile the pc build and run it in interactive mode"
	@echo "  check    - check project's C++ formatting using clang-format"
	@echo "  format   - format project's C++ code using clang-format"
	@echo "  version  - update version in globals.hpp based on current git state"
	@echo "  release  - tag and push a release"

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

check:
	clang-format --dry-run -Werror -i src-common/*.cpp src-common/*.hpp src-pc/*.cpp src-pc/*.hpp

format:
	clang-format -i src-common/*.cpp src-common/*.hpp src-pc/*.cpp src-pc/*.hpp

version: CUR_VERSION=$(shell sed -nr '/^#define ANT_VERSION/{ s/^.*"([^"]+)"/\1/p }' src-common/globals.hpp)
version: VERSION:=$(shell git describe --exact-match 2>/dev/null | grep -Po "(?<=^v)\d+\.\d+\.\d+$$" || git rev-parse --short=7 HEAD)

version:
	@echo "Current version: $(CUR_VERSION)"
	@echo "New version: $(VERSION)"
	@echo "Updating src-common/globals.hpp with the new version."
	@sed -i "s/^#define ANT_VERSION.*/#define ANT_VERSION \"${VERSION}\"/g" src-common/globals.hpp

release:
	@if [ ! "$(VERSION)" ]; then echo; echo "Please specify the release version like this:"; echo '  make release VERSION=2.0.1'; echo; exit 1; fi
	@if ! echo "$(VERSION)" | grep -qP "^\d+\.\d+\.\d+$$"; then echo "ERROR: VERSION must be x.y.z where x,y,z are all digits"; exit 1; fi
	@make version VERSION=$(VERSION)
	@git tag -a -m v$(VERSION) v$(VERSION)
	@git push origin v$(VERSION)
