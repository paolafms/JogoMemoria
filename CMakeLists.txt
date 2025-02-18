# CMake Minimum Version
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(JogoMemoria C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(JogoMemoria JogoMemoria.c lib/ssd1306.c)

# Include directories for the project
target_include_directories(JogoMemoria PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Biblioteca padrão do Raspberry Pi Pico
target_link_libraries(JogoMemoria
        pico_stdlib
)

# Suporte à I2C (hardware/i2c.h)
target_link_libraries(JogoMemoria
        hardware_i2c
)

# Biblioteca PIO para WS2812
target_link_libraries(JogoMemoria
    hardware_pio
)

target_link_libraries(JogoMemoria
    hardware_adc
)

# PWM (hardware/pwm.h)
target_link_libraries(JogoMemoria
    hardware_pwm
)

pico_generate_pio_header(JogoMemoria ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_include_directories(JogoMemoria PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(JogoMemoria)
