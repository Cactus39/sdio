# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(sdio_card_reader C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(sdio_card_reader sdio_card_reader.c tusb.c usb_descriptors.c sdio_utils.c)

pico_set_program_name(sdio_card_reader "sdio_card_reader")
pico_set_program_version(sdio_card_reader "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(sdio_card_reader 1)
pico_enable_stdio_usb(sdio_card_reader 0)

# Add the standard library to the build
target_link_libraries(sdio_card_reader
        pico_stdlib
        tinyusb_device
        tinyusb_board
        hardware_pio)

# Add the standard include files to the build
target_include_directories(sdio_card_reader PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(sdio_card_reader 
        
        )

pico_add_extra_outputs(sdio_card_reader)

pico_generate_pio_header(sdio_card_reader ${CMAKE_CURRENT_LIST_DIR}/cmd_asm.pio)
pico_generate_pio_header(sdio_card_reader ${CMAKE_CURRENT_LIST_DIR}/dat_asm.pio)