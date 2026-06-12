
PROJECT_NAME = sdio_card_reader


PICO_SDK_PATH_DEFAULT = $(HOME)/.pico-sdk/sdk/2.2.0
PICO_TOOLCHAIN_PATH_DEFAULT = $(HOME)/.pico-sdk/toolchain/14_2_Rel1

export PICO_SDK_PATH ?= $(PICO_SDK_PATH_DEFAULT)
export PICO_TOOLCHAIN_PATH ?= $(PICO_TOOLCHAIN_PATH_DEFAULT)


OPENOCD_EXT := $(wildcard $(HOME)/.pico-sdk/openocd/*/bin/openocd)
OPENOCD ?= $(if $(OPENOCD_EXT),$(OPENOCD_EXT),openocd)

BUILD_DIR = build

.PHONY: all clean flash configure

all: configure
	@cmake --build $(BUILD_DIR) -j$(shell nproc)

configure:
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		cmake -B $(BUILD_DIR) -G "Unix Makefiles" \
			-DPICO_SDK_PATH=$(PICO_SDK_PATH) \
			-DPICO_TOOLCHAIN_PATH=$(PICO_TOOLCHAIN_PATH); \
	fi

flash: all
	@echo "=== Raspberry Pi Debug Probe ==="
	$(OPENOCD) -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "program $(BUILD_DIR)/$(PROJECT_NAME).elf verify reset exit"

clean:
	rm -rf $(BUILD_DIR)
