BUILD_TYPE?=Release
BUILD_DIR?=build

#
# Traditional build commands / make interface
#
default: make

.PHONY: debug
debug:
	$(eval BUILD_TYPE := Debug)

.PHONY: cmake_check
cmake_check:
	@cmake --version || (echo "\n** Please install 'cmake' **\n" && exit 1)

.PHONY: configure
configure: cmake_check
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ../src
	@echo "Modify build configuration in '$(BUILD_DIR)'"

.PHONY: make
make: configure
	cd $(BUILD_DIR) && make

.PHONY: install
install:
	cd $(BUILD_DIR) && make install

.PHONY: clean
clean:
	rm -fr $(BUILD_DIR) || true

all: clean default install

#
# Extension: package generation
#
.PHONY: make-pkg
make-pkg: configure
	cd $(BUILD_DIR) && make package

.PHONY: install-pkg
install-pkg:
	sudo dpkg -i $(BUILD_DIR)/*.deb

.PHONY: uninstall-pkg
uninstall-pkg:
	sudo apt-get --yes remove nvm_eraser || true

#
# Extension: commands useful in development cycle
#
.PHONY: dev
dev: uninstall-pkg clean make make-pkg install-pkg
