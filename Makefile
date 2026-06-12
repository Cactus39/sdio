# Имя вашего исполняемого файла (программы) из CMakeLists.txt
PROJECT_NAME = sdio_card_reader

# Стандартные пути, куда расширение VScodium установило SDK и компилятор
PICO_SDK_PATH_DEFAULT = $(HOME)/.pico-sdk/sdk/2.2.0
PICO_TOOLCHAIN_PATH_DEFAULT = $(HOME)/.pico-sdk/toolchain/14_2_Rel1

# Экспортируем переменные для CMake (если они не заданы глобально в системе)
export PICO_SDK_PATH ?= $(PICO_SDK_PATH_DEFAULT)
export PICO_TOOLCHAIN_PATH ?= $(PICO_TOOLCHAIN_PATH_DEFAULT)

# Автоматически находим утилиту OpenOCD от расширения или берем системную
OPENOCD_EXT := $(wildcard $(HOME)/.pico-sdk/openocd/*/bin/openocd)
OPENOCD ?= $(if $(OPENOCD_EXT),$(OPENOCD_EXT),openocd)

# Временная папка для сборки
BUILD_DIR = build

.PHONY: all clean flash configure

# Цель по умолчанию: запустить конфигурацию и скомпилировать проект
all: configure
	@cmake --build $(BUILD_DIR) -j$(shell nproc)

# Автоматическая настройка CMake (генерирует промежуточные Makefile-файлы)
configure:
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		cmake -B $(BUILD_DIR) -G "Unix Makefiles" \
			-DPICO_SDK_PATH=$(PICO_SDK_PATH) \
			-DPICO_TOOLCHAIN_PATH=$(PICO_TOOLCHAIN_PATH); \
	fi

# Прошивка платы через Raspberry Pi Debug Probe (по интерфейсу SWD)
flash: all
	@echo "=== Запуск прошивки через Raspberry Pi Debug Probe ==="
	$(OPENOCD) -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "program $(BUILD_DIR)/$(PROJECT_NAME).elf verify reset exit"

# Полная очистка проекта (удаление временных файлов сборки)
clean:
	rm -rf $(BUILD_DIR)
